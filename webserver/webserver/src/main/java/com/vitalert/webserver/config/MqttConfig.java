package com.vitalert.webserver.config;

import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
import org.springframework.beans.factory.annotation.Autowired;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.vitalert.webserver.service.JDBCService;
import com.fasterxml.jackson.core.JsonProcessingException;

public class MqttConfig implements MqttCallback {

    private MqttClient mqttClient;

    @Autowired
    JDBCService jdbcService;

    public MqttConfig(String broker, String clientId) throws MqttException {
        mqttClient = new MqttClient(broker, clientId, new MemoryPersistence());
        mqttClient.setCallback(this); // Define esta instância como o callback
        connect();
    }

    private void connect() throws MqttException {
        MqttConnectOptions options = new MqttConnectOptions();
        options.setCleanSession(true); // Limpar sessão anterior
        mqttClient.connect(options);
    }

    public void subscribe(String topic) throws MqttException {
        mqttClient.subscribe(topic);
    }

    @Override
    public void connectionLost(Throwable cause) {
        System.out.println("Conexão MQTT perdida. Tentando reconectar...");
        try {
            connect();
        } catch (MqttException e) {
            System.out.println("Falha ao reconectar: " + e.getMessage());
        }
    }

    @Override
    public void messageArrived(String topic, MqttMessage message) throws Exception {

        String deviceMessage = new String(message.getPayload());
        System.out.println("Nova mensagem recebida:" + deviceMessage);
        ObjectMapper objectMapper = new ObjectMapper();

        try {
            // Transforma a string JSON em um JsonNode
            JsonNode jsonNode = objectMapper.readTree(deviceMessage);

            int messageType = jsonNode.get("messageType").asInt();

            switch(messageType) {
                case 0:
                    int valid = jsonNode.get("valid").asInt();
                    int bpm = jsonNode.get("bpm").asInt();
                    float spo2 = jsonNode.get("spo2").floatValue();
                    System.out.println("Tipo de mensagem: 0");
                    System.out.println("Valid: " + valid);
                    System.out.println("BPM: " + bpm);
                    System.out.println("SpO2: " + spo2);
                    if(valid == 1) {
                        jdbcService.saveBpm(bpm);
                        jdbcService.saveSpo2(spo2);
                    }
                    break;
                case 1:
                    float temperature = jsonNode.get("temperature").floatValue();
                    System.out.println("Tipo de mensagem: 1");
                    System.out.println("Temperature: " + temperature);
                    jdbcService.saveTemperature(temperature);
                    break;
                case 2:
                    boolean fallDetected = jsonNode.get("fallDetected").asBoolean();
                    System.out.println("Tipo de mensagem: 2");
                    System.out.println("Fall detected: " + fallDetected);
                    jdbcService.saveFallDetection();
                    break;
                case 3:
                    boolean buttonPressed = jsonNode.get("buttonPressed").asBoolean();
                    System.out.println("Tipo de mensagem: 3");
                    System.out.println("Button pressed: " + buttonPressed);
                    jdbcService.savePanicAlert();
                    break;
                default:
                    System.out.println("Tipo de mensagem desconhecido");
            }
        } catch (JsonProcessingException e) {
            System.out.println("Erro ao transformar JSON em objeto: " + e.getMessage());
        }
        System.out.println("========================================");
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken token) {
        // Ação opcional após a entrega da mensagem
    }

    public void disconnect() throws MqttException {
        mqttClient.disconnect();
    }
}
