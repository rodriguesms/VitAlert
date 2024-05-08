package com.vitalert.webserver.service;

import org.eclipse.paho.client.mqttv3.MqttException;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

import com.vitalert.webserver.config.MqttConfig;

import javax.annotation.PostConstruct;
import javax.annotation.PreDestroy;

@Component
public class MqttService {

    @Value("${mqtt.broker.url}")
    private String mqttBroker;

    @Value("${mqtt.broker.clientId}")
    private String mqttClientId;

    @Value("${mqtt.broker.topic}")
    private String mqttTopic;

    private MqttConfig mqttConfig;

    @PostConstruct
    public void initialize() {
        try {
            mqttConfig = new MqttConfig(mqttBroker, mqttClientId);
            mqttConfig.subscribe(mqttTopic); // Subscreva aos tópicos desejados
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    @PreDestroy
    public void cleanup() {
        try {
            mqttConfig.disconnect();
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }
}
