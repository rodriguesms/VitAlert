#include <Wire.h>
#include <MPU6050_tockn.h>
#include "MAX30105.h"           //MAX3010x library
#include "heartRate.h"          //Heart rate calculating algorithm
#include <OneWire.h>
#include <DallasTemperature.h>

// *************************************** Panic button ***************************************
#define BUTTON_PIN 25

int lastState = HIGH; // the previous state from the input pin
int currentState;     // the current reading from the input pin

void setup_panic_button() {
  // initialize the pushbutton pin as an pull-up input
  // the pull-up input pin will be HIGH when the switch is open and LOW when the switch is closed.
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop_panic_button() {
  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);

  if(lastState == LOW && currentState == HIGH)
    Serial.println("The state changed from LOW to HIGH");

  // save the last state
  lastState = currentState;
}

// ************************************** Fall detection **************************************
#define I2C2_SDA_PIN 5
#define I2C2_SCL_PIN 18

TwoWire i2cBus1 = TwoWire(1); //i2cBus1 bus
MPU6050 mpu6050(i2cBus1);

void setup_fall_detection() {
  i2cBus1.begin(I2C2_SDA_PIN, I2C2_SCL_PIN, 100000);
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
}

void loop_fall_detection() {
  mpu6050.update();

  float accelX = mpu6050.getAccX();
  float accelY = mpu6050.getAccY();
  float accelZ = mpu6050.getAccZ();

  float totalAccel = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

  const float fallThreshold = 2; // You might need to adjust this value
  
  // Check for a fall
  if (totalAccel > fallThreshold) {
    // Possible fall detected, do something
    Serial.println("Fall detected!");
  }
}

// **************************************** Heart rate ****************************************
#define I2C1_SDA_PIN 21
#define I2C1_SCL_PIN 22

TwoWire i2cBus0 = TwoWire(0); //i2cBus0 bus

MAX30105 particleSensor;

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

double SpO2 = 0;
double ESpO2 = 90.0;
double FSpO2 = 0.7; //filter factor for estimated SpO2
double frate = 0.95; //low pass filter for IR/red LED value to eliminate AC component
int i = 0;
int Num = 30;
#define FINGER_ON 7000
#define MINIMUM_SPO2 90.0

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

void loop_heart_rate() {
  long irValue = particleSensor.getIR();    //Reading the IR value it will permit us to know if there's a finger on the sensor or not
  if (irValue > FINGER_ON ) {
    
    if (checkForBeat(irValue) == true) {
      Serial.print("Bpm="); Serial.println(beatAvg);
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
      Serial.println("red=" + String(red) + ",IR=" + String(ir) + ",i=" + String(i));
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
        Serial.print(",SPO2="); Serial.println(ESpO2);
        sumredrms = 0.0; sumirrms = 0.0; SpO2 = 0;
        i = 0;
      }
      particleSensor.nextSample(); //We're finished with this sample so move to next sample
    }
    
    Serial.print("Bpm:" + String(beatAvg));
    if (beatAvg > 30)  Serial.println(",SPO2:" + String(ESpO2));
    else Serial.println(",SPO2:" + String(ESpO2));
  }
  else {
    for (byte rx = 0 ; rx < RATE_SIZE ; rx++) rates[rx] = 0;
    beatAvg = 0; rateSpot = 0; lastBeat = 0;
    avered = 0; aveir = 0; sumirrms = 0; sumredrms = 0;
    SpO2 = 0; ESpO2 = 90.0;
  }
}

// **************************************** Temperature ***************************************
// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

void setup_temperature() {
  // Start the DS18B20 sensor
  sensors.begin();
  sensors.setWaitForConversion(0);
}

void loop_temperature() {
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  Serial.print(temperatureC);
  Serial.println("ºC");
}

void setup() {
  Serial.begin(115200);
  Serial.println("System Start");

  setup_panic_button();
  setup_fall_detection();
  setup_heart_rate();
  setup_temperature();
}

void loop() {
  loop_panic_button();
  loop_fall_detection();
  loop_heart_rate();
  loop_temperature();
}
