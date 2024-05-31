package com.vitalert.webserver.service;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.stereotype.Service;

import com.vitalert.webserver.config.JDBCConector;

@Service
public class JDBCService extends JDBCConector {

    @Autowired
    private JdbcTemplate jdbcTemplate;

    public Map<String, List<Map<String, Object>>> getLastRecords() {
        Map<String, List<Map<String, Object>>> result = new HashMap<>();

        result.put("temperature", getLastRecordsForTable("temperature"));
        result.put("fall_detection", getLastRecordsForTable("fall_detection"));
        result.put("panic_alert", getLastRecordsForTable("panic_alert"));
        result.put("spo2", getLastRecordsForTable("spo2"));
        result.put("bpm", getLastRecordsForTable("bpm"));

        return result;
    }

    private List<Map<String, Object>> getLastRecordsForTable(String tableName) {
        String sql = "SELECT * FROM " + tableName + " ORDER BY reported_at DESC LIMIT 10";
        return jdbcTemplate.queryForList(sql);
    }

    public void saveFallDetection(){
        String saveFallDetectionSql = "INSERT INTO fall_detection DEFAULT VALUES";
        try (Connection conn = connect();
            PreparedStatement statement = conn.prepareStatement(saveFallDetectionSql, PreparedStatement.RETURN_GENERATED_KEYS)) {
                statement.executeUpdate();
            }catch (SQLException ex) {
                ex.printStackTrace();
            }
    }

    public void saveTemperature(float reportedValue) {
        String saveTemperatureSql = "INSERT INTO temperature (reported_value) VALUES (?)";
        try (Connection conn = connect();
            PreparedStatement statement = conn.prepareStatement(saveTemperatureSql, PreparedStatement.RETURN_GENERATED_KEYS)) {
            
            statement.setFloat(1, reportedValue);
            statement.executeUpdate();

            try (ResultSet generatedKeys = statement.getGeneratedKeys()) {
                if (generatedKeys.next()) {
                    long id = generatedKeys.getLong(1);
                    System.out.println("Temperatura salva com ID: " + id);
                } else {
                    System.err.println("Falha ao obter ID da temperatura.");
                }
            }
        } catch (SQLException ex) {
            ex.printStackTrace();
        }
    }

    public void savePanicAlert() {
        String savePanicAlertSql = "INSERT INTO panic_alert DEFAULT VALUES";
        try (Connection conn = connect();
             PreparedStatement statement = conn.prepareStatement(savePanicAlertSql, PreparedStatement.RETURN_GENERATED_KEYS)) {
            
            statement.executeUpdate();
    
            try (ResultSet generatedKeys = statement.getGeneratedKeys()) {
                if (generatedKeys.next()) {
                    long id = generatedKeys.getLong(1);
                    System.out.println("Alerta de pânico salvo com ID: " + id);
                } else {
                    System.err.println("Falha ao obter ID do alerta de pânico.");
                }
            }
        } catch (SQLException ex) {
            ex.printStackTrace();
        }
    }

    public void saveSpo2(float reportedValue) {
        String saveSpo2Sql = "INSERT INTO spo2 (reported_value) VALUES (?)";
        try (Connection conn = connect();
             PreparedStatement statement = conn.prepareStatement(saveSpo2Sql, PreparedStatement.RETURN_GENERATED_KEYS)) {
            
            statement.setFloat(1, reportedValue);
            statement.executeUpdate();
    
            try (ResultSet generatedKeys = statement.getGeneratedKeys()) {
                if (generatedKeys.next()) {
                    long id = generatedKeys.getLong(1);
                    System.out.println("SpO2 salvo com ID: " + id);
                } else {
                    System.err.println("Falha ao obter ID do SpO2.");
                }
            }
        } catch (SQLException ex) {
            ex.printStackTrace();
        }
    }

    public void saveBpm(int reportedValue) {
        String saveBpmSql = "INSERT INTO bpm (reported_value) VALUES (?)";
        try (Connection conn = connect();
             PreparedStatement statement = conn.prepareStatement(saveBpmSql, PreparedStatement.RETURN_GENERATED_KEYS)) {
            
            statement.setInt(1, reportedValue);
            statement.executeUpdate();
    
            try (ResultSet generatedKeys = statement.getGeneratedKeys()) {
                if (generatedKeys.next()) {
                    long id = generatedKeys.getLong(1);
                    System.out.println("BPM salvo com ID: " + id);
                } else {
                    System.err.println("Falha ao obter ID do BPM.");
                }
            }
        } catch (SQLException ex) {
            ex.printStackTrace();
        }
    }    

}
