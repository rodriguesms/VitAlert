package com.vitalert.webserver.service;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.stereotype.Service;

@Service
public class DataService {

    @Autowired
    private JdbcTemplate jdbcTemplate;

    public void insertData()
    
}
