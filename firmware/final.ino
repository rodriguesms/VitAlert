#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <MPU6050_tockn.h>
#include "heartRate.h"
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// Constants for Wi-Fi and MQTT connection
const char* ssid = "CLAUDIA";
const char* password = "claudia1303";
const char* mqtt_broker = "192.168.15.127";
const char* topic = "esp32/test";
const char* mqtt_username = "";
const char* mqtt_password = "";
const int mqtt_port = 1883;

// WiFi and MQTT Clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Declare sensor object variables
TwoWire i2cBus1 = TwoWire(1); // Fall detection
TwoWire i2cBus0 = TwoWire(0); // Heart Rate
MPU6050 mpu6050(i2cBus1);
MAX30105 particleSensor;
uint32_t irValue;
OneWire oneWire(4); // Temperature
DallasTemperature sensors(&oneWire);

// Button pin setup
#define BUTTON_PIN 25
int lastButtonState = HIGH;

// MPU6050 setup
#define I2C2_SDA_PIN 5
#define I2C2_SCL_PIN 18

// MAX30105 setup
#define I2C1_SDA_PIN 21
#define I2C1_SCL_PIN 22

// Beat detection and SpO2 variables
byte pulseLED = 0, lastPulseLED = 2;    // The on-board LED

const byte RATE_SIZE = 16;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;

double avered = 0;
double aveir = 0;
double sumirrms = 0;
double sumredrms = 0;

String hr_data;
long delta_message;
long last_beat_message = 0;

double SpO2 = 0;
double ESpO2 = 90.0;
double FSpO2 = 0.7; //filter factor for estimated SpO2
double frate = 0.95; //low pass filter for IR/red LED value to eliminate AC component
int i = 0;
int Num = 30;
#define FINGER_ON 7000
#define MINIMUM_SPO2 90.0

// Panic button variables
int currentState; // the current reading from the input pin

// Temperature variables
const int oneWireBus = 4;

// Sensor setup functions
void setupSensors();

// MQTT Connection function
void connectToMQTT();

// Functions for sensor readings
float readTemperature();
String loop_heart_rate();
float readFallDetectionAcceleration();
int readButton();

int button_check = 0;
int button_val = 0;

// Task function to publish sensor data
void sensorReadingTask(void *parameter);

int fall_check = 0;
int fall_val = 0;

String serializeSensorData(float temperature);

void setup() {
  Serial.begin(115200);
  Serial.println("aqui pelo menos");
  // Initialize sensors
  setupSensors();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  // Connect to MQTT Broker
  mqttClient.setServer(mqtt_broker, mqtt_port);
  connectToMQTT();
  
  // Start FreeRTOS task for sensor reading and MQTT publishing
  xTaskCreate(sensorReadingTask, "SensorReadingTask", 4096, NULL, 1, NULL);
}

void loop() {
  // Empty loop - task handles sensor readings and data publishing
  hr_data = loop_heart_rate();
  delta_message = millis() - last_beat_message;
  fall_val = readFallDetectionAcceleration();
  if (fall_val){
    fall_check = 1;
  }

  button_val = readButton();
  if (button_val){
    button_check = 1;
  }

  if (delta_message > 5000 && mqttClient.connected()) {
      if (fall_check){
        String fall_message = "{\"messageType\":2, \"fallDetected\": 1}";
        mqttClient.publish(topic, fall_message.c_str());
        Serial.println("MQTT Data Sent: " + fall_message);
        fall_check = 0;
      }

      if (button_check){
        String button_message = "{\"messageType\":3, \"buttonPressed\": 1}";
        mqttClient.publish(topic, button_message.c_str());
        Serial.println("MQTT Data Sent: " + button_message);
        button_check = 0;
      }

      last_beat_message = millis();
      mqttClient.publish(topic, hr_data.c_str());
      Serial.println("MQTT Data Sent: " + hr_data);
  }
}

void setup_heart_rate() {
  delay(3000);

  i2cBus0.begin(I2C1_SDA_PIN, I2C1_SCL_PIN);
  if (!particleSensor.begin(i2cBus0, I2C_SPEED_FAST)) //400kHz speed
  {
    Serial.println("MAX30102 NOT INITIALIZED");
    while (1);
  }
  byte ledBrightness = 0x7F; //=127, Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR
  int sampleRate = 800; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 215; //Options: 69, 118, 215, 411
  int adcRange = 16384; //Options: 2048, 4096, 8192, 16384
  // Set up the wanted parameters
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
  particleSensor.enableDIETEMPRDY();

  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

String loop_heart_rate() {
  long irValue = particleSensor.getIR();    //Reading the IR value it will permit us to know if there's a finger on the sensor or not
  if (irValue > FINGER_ON ) {
    
    if (checkForBeat(irValue) == true) {
      //Serial.print("Bpm="); Serial.println(beatAvg);
      long delta = millis() - lastBeat;
      lastBeat = millis();
      beatsPerMinute = 60 / (delta / 1000.0);
      if (beatsPerMinute < 255 && beatsPerMinute > 20) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE ; x++) beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }

    uint32_t ir, red ;
    double fred, fir;
    particleSensor.check(); //Check the sensor, read up to 3 samples
    if (particleSensor.available()) {
      i++;
      ir = particleSensor.getFIFOIR();
      red = particleSensor.getFIFORed();
      //Serial.println("red=" + String(red) + ",IR=" + String(ir) + ",i=" + String(i));
      fir = (double)ir;//double
      fred = (double)red;//double
      aveir = aveir * frate + (double)ir * (1.0 - frate); //average IR level by low pass filter
      avered = avered * frate + (double)red * (1.0 - frate);//average red level by low pass filter
      sumirrms += (fir - aveir) * (fir - aveir);//square sum of alternate component of IR level
      sumredrms += (fred - avered) * (fred - avered); //square sum of alternate component of red level

      if ((i % Num) == 0) {
        double R = (sqrt(sumirrms) / aveir) / (sqrt(sumredrms) / avered);
        SpO2 = -23.3 * (R - 0.4) + 100;
        ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2;//low pass filter
        if (ESpO2 <= MINIMUM_SPO2) ESpO2 = MINIMUM_SPO2; //indicator for finger detached
        if (ESpO2 > 100) ESpO2 = 99.9;
        //Serial.print(",SPO2="); Serial.println(ESpO2);
        sumredrms = 0.0; sumirrms = 0.0; SpO2 = 0;
        i = 0;
      }
      particleSensor.nextSample(); //We're finished with this sample so move to next sample
    }
    
    //Serial.print("Bpm:" + String(beatAvg));
    //if (beatAvg > 30)  Serial.println(",SPO2:" + String(ESpO2));
    //else Serial.println(",SPO2:" + String(ESpO2));
  }
  else {
    for (byte rx = 0 ; rx < RATE_SIZE ; rx++) rates[rx] = 0;
    beatAvg = 0; rateSpot = 0; lastBeat = 0;
    avered = 0; aveir = 0; sumirrms = 0; sumredrms = 0;
    SpO2 = 0; ESpO2 = 90.0;
  }
  String hrData = "";
  if (irValue > FINGER_ON) { // Detect if finger is in the sensor
    // Algorithm to read heart rate and SpO2 levels. Add your algorithm here.
    hrData = "{\"messageType\":0,\"valid\":1,\"bpm\":"+String(beatAvg)+",\"spo2\":"+String(ESpO2) + "}"; // Placeholder for sample values
  } else {
    hrData = "{\"messageType\":0,\"valid\":0,\"bpm\":0,\"spo2\":0}"; // No finger detected
  }
  return hrData;
}

void sensorReadingTask(void *parameter) {
  while (1) {
    if (!mqttClient.connected()) {
       connectToMQTT();
    }
    // Get sensor readings
    float temp = readTemperature();

    String sensorData = serializeSensorData(temp);

    // Publish sensor data to MQTT
    if (mqttClient.connected()) {
      mqttClient.publish(topic, sensorData.c_str());
      Serial.println("MQTT Data Sent: " + sensorData);
    }

    // Delay to control how often you read sensors and send data
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void setupSensors() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.println("butao");
  i2cBus1.begin(I2C2_SDA_PIN, I2C2_SCL_PIN, 100000);
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  Serial.println("girou");
  i2cBus0.begin(I2C1_SDA_PIN, I2C1_SCL_PIN, 400000);
  particleSensor.begin(i2cBus0, I2C_SPEED_FAST); // Use default I2C port, 400kHz speed
  particleSensor.setup();
  Serial.println("particulou");
  oneWire.begin(oneWireBus);
  setup_heart_rate();
  sensors.begin();
  Serial.println("setou");
}

void connectToMQTT() {
  while (!mqttClient.connected()) {
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT Broker!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
  }
}

String serializeSensorData(float temperature) {
  String data = "{\"messageType\":1,";
  data += "\"temperature\":" + String(temperature) + ",";
  //data += "\"heartRate\":" + heartRateData + ",";
  //data += "\"fallDetected\":" + String(fallAccel) + ",";
  data += "}";
  return data;
}

int readButton() {
  currentState = digitalRead(BUTTON_PIN);
  bool pressed = (lastButtonState == HIGH && currentState == LOW); // Check for button press (active LOW)
  lastButtonState = currentState; // Update the last button state
  return pressed;
}

float readFallDetectionAcceleration() {
  mpu6050.update();

  float accelX = mpu6050.getAccX();
  float accelY = mpu6050.getAccY();
  float accelZ = mpu6050.getAccZ();

  float totalAccel = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

  const float fallThreshold = 2; // You might need to adjust this value
  
  // Check for a fall
  if (totalAccel > fallThreshold) {
    // Possible fall detected, do something
    return 1;
  }
  else{
    return 0;
  }
}

float readTemperature() {
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  return temp;
}
