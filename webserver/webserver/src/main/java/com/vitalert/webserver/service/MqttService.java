package com.vitalert.webserver.data;

import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import com.vitalert.webserver.config.MqttConfig;

import jakarta.annotation.PostConstruct;
import jakarta.annotation.PreDestroy;

@Service
public class DataService {
    
    private MqttClient mqttClient;

    @Autowired
    private MqttConfig mqttConfig;

    @PostConstruct
    public void init() throws MqttException {
        mqttClient = mqttConfig.mqttClient();
    }

    @PreDestroy
    public void destroy() throws MqttException {
        if(mqttClient != null && mqttClient.isConnected())
            mqttClient.disconnect();
    }

}
