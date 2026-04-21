#include "esp32_wifi.hpp"
#include "mod_dht22.h"
#include "mod_eeprom.h"
#include "mod_air780e.h"          // ← 改回 air780e
#include "mod_dd_calc.h"

extern uint8_t cmd_flag;
extern char data_buff[50];

uint32_t lastSample      = 0;
uint32_t lastStatusPrint = 0;

void setup() {
    serial_init();
    DHT22_Init();
    EEPROM_Init();

    Serial.println("=== STM32 PestMonitor ===");

    Serial.println("[ESP32-CAM] 模块上电等待...");
    delay(2000);
    SET_ESP_WIFI_MODE();
    SET_STA_WIFI();
    SET_AP_WIFI();
    SET_ESP_AI_MODE(Nornal_AI);
    cmd_flag = 2;

    Get_STAIP(); delay(2000);
    Serial.print("[STA IP] "); Serial.println(data_buff);
    Get_APIP();  delay(2000);
    Serial.print("[AP  IP] "); Serial.println(data_buff);

    float savedDD = EEPROM_LoadDD();
    DD_Init(savedDD);

    Air780E_Init();                // ← 改回 Air780E

    lastSample      = millis();
    lastStatusPrint = millis();
    Serial.println("=== 开始监测 ===");
}

void loop() {
    recv_data();

    if (millis() - lastSample >= SAMPLE_INTERVAL) {
        lastSample = millis();
        if (DHT22_Read()) {
            Air780E_CheckTHAlert(gtemp, ghumi);   // ← 改回 Air780E
            DD_Update(gtemp);
        }
    }

    if (millis() - lastStatusPrint >= 60000) {
        lastStatusPrint = millis();
        DD_PrintStatus();
    }
}