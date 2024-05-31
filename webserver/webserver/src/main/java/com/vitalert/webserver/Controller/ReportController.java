package com.vitalert.webserver.Controller;

import java.util.List;
import java.util.Map;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import com.vitalert.webserver.service.JDBCService;

@RestController
public class ReportController {

    @Autowired
    private JDBCService dataService;
    
    @GetMapping("/last-records")
    public ResponseEntity<Map<String, List<Map<String, Object>>>> getLastRecords() {
        return ResponseEntity.ok(dataService.getLastRecords());
    }

}
