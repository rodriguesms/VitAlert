package com.vitalert.webserver.config;

import org.eclipse.paho.client.mqttv3.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;


@Configuration
public class MqttConfig {
    private static final Logger logger = LoggerFactory.getLogger(MqttConfig.class);

    @Bean
    public MqttClient mqttClient() throws MqttException {
    
        MqttClient mqttClient = new MqttClient("tcp://localhost:1883", MqttClient.generateClientId());
        
        mqttClient.setCallback(new MqttCallback() {
            @Override
            public void connectionLost(Throwable cause) {
                logger.error("Connection Lost.", cause);
            }

            @Override
            public void messageArrived(String topic, MqttMessage message) {
                logger.info("Message arrived.", message.getPayload());
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {
                logger.info("Delivery complete.", token);
            }
        });

        mqttClient.connect();
        mqttClient.subscribe("seu/topico/aqui", 0); // Subscreva ao tópico desejado

        return mqttClient;
    }
}
