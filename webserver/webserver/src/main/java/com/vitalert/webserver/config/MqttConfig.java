package com.vitalert.webserver.config;

import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

public class MqttConfig implements MqttCallback {

    private MqttClient mqttClient;

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
        System.out.println("Nova mensagem recebida:");
        System.out.println("Tópico: " + topic);
        System.out.println("Conteúdo: " + new String(message.getPayload()));

        // Implemente a lógica de processamento da mensagem aqui
        // Por exemplo, chamar métodos ou serviços, atualizar dados, etc.
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken token) {
        // Ação opcional após a entrega da mensagem
    }

    public void disconnect() throws MqttException {
        mqttClient.disconnect();
    }
}
