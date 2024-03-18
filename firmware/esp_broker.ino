#include <WiFi.h>
#include <PubSubClient.h>

// WiFi
const char *ssid = "1702A"; // Enter your WiFi name
const char *password = "josefilho2016";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "esp32/test";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// Publish function every 5 seconds
void publishMessage(void *parameter) {
  while (1) {
    if (client.connected()) {
      client.publish(topic, "teste");
    } else {
      Serial.println("Desconectado do MQTT");
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi");
  }
  Serial.println("Connected to the WiFi network");
  client.setServer(mqtt_broker, mqtt_port);
  
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.subscribe(topic);

  // Create a task that will be responsible for publishing messages
  // This function will use rtos, so it works separately from the main loop function
  xTaskCreate(
    publishMessage,  // Function that should be called
    "PublishMessage",  // Name of the task
    2048,             // Stack size (bytes)
    NULL,             // Parameter to pass to the function
    1,                // Task priority
    NULL              // Task handle
  );
}

void loop() {
  client.loop();
}