#include <Arduino.h>
#include "mod_air780e.h"

static bool moduleReady         = false;
static uint8_t consecAlertCount = 0;
static bool thAlertSent         = false;

static bool sendAT(const char* cmd, const char* expect, uint32_t timeout = 5000) {
    while (AIR780E_SERIAL.available()) AIR780E_SERIAL.read();
    AIR780E_SERIAL.println(cmd);
    Serial.print("[AT→] "); Serial.println(cmd);

    String resp = "";
    uint32_t t = millis();
    while (millis() - t < timeout) {
        while (AIR780E_SERIAL.available()) {
            resp += (char)AIR780E_SERIAL.read();
        }
        if (resp.indexOf(expect) >= 0) {
            Serial.print("[AT←] "); Serial.println(resp);
            return true;
        }
        if (resp.indexOf("ERROR") >= 0) {
            Serial.print("[AT←ERR] "); Serial.println(resp);
            return false;
        }
    }
    Serial.print("[AT←TIMEOUT] "); Serial.println(resp);
    return false;
}

bool Air780E_Init() {
    AIR780E_SERIAL.begin(AIR780E_BAUDRATE);
    Serial.println("[Air780E] 初始化，等待模块上电...");
    delay(3000);

    // 1. 握手检测
    bool alive = false;
    for (int i = 0; i < 5; i++) {
        if (sendAT("AT", "OK", 2000)) { alive = true; break; }
        Serial.print("[Air780E] 握手重试 "); Serial.println(i + 1);
        delay(1000);
    }
    if (!alive) {
        Serial.println("[Air780E] ❌ 无响应，检查PA9/PA10接线和供电");
        return false;
    }

    // 2. 关回显
    sendAT("ATE0", "OK", 2000);

    // 3. 检查SIM卡
    if (!sendAT("AT+CPIN?", "READY", 5000)) {
        Serial.println("[Air780E] ❌ SIM卡未就绪，检查SIM卡是否插好");
        return false;
    }

    // 4. 等待网络注册（最多30秒）
    Serial.println("[Air780E] 等待网络注册...");
    bool netOk = false;
    for (int i = 0; i < 10; i++) {
        AIR780E_SERIAL.println("AT+CREG?");
        delay(2000);
        String r = "";
        uint32_t t = millis();
        while (millis() - t < 1000) {
            while (AIR780E_SERIAL.available()) r += (char)AIR780E_SERIAL.read();
        }
        Serial.print("[CREG] "); Serial.println(r);
        if (r.indexOf(",1") >= 0 || r.indexOf(",5") >= 0) {
            netOk = true;
            Serial.println("[Air780E] ✅ 网络注册成功");
            break;
        }
        Serial.print("[Air780E] 等待网络 "); Serial.print(i + 1); Serial.println("/10");
    }
    if (!netOk) {
        Serial.println("[Air780E] ❌ 网络注册失败，检查SIM卡信号");
        return false;
    }

    // 5. 设置短信文本模式
    if (!sendAT("AT+CMGF=1", "OK", 3000)) return false;

    // 6. 设置字符集
    if (!sendAT("AT+CSCS=\"GSM\"", "OK", 3000)) return false;

    moduleReady = true;
    Serial.println("[Air780E] ✅ 初始化完成，发送启动短信");

    // 7. 启动短信
    Air780E_SendSMS("PestMonitor system started. Ready to monitor.");
    return true;
}

bool Air780E_SendSMS(const char* content) {
    if (!moduleReady) {
        Serial.println("[Air780E] 模块未就绪，跳过短信");
        return false;
    }

    char cmd[40];
    sprintf(cmd, "AT+CMGS=\"%s\"", SMS_TARGET);

    if (!sendAT("AT+CMGF=1", "OK", 2000)) return false;
    if (!sendAT(cmd, ">", 8000)) {
        Serial.println("[Air780E] ❌ 未收到>提示符");
        return false;
    }

    AIR780E_SERIAL.print(content);
    AIR780E_SERIAL.write(0x1A);

    Serial.print("[Air780E] 发送内容: "); Serial.println(content);

    String resp = "";
    uint32_t t = millis();
    while (millis() - t < 15000) {
        while (AIR780E_SERIAL.available()) resp += (char)AIR780E_SERIAL.read();
        if (resp.indexOf("+CMGS:") >= 0) {
            Serial.println("[Air780E] ✅ 短信发送成功");
            return true;
        }
        if (resp.indexOf("ERROR") >= 0) {
            Serial.println("[Air780E] ❌ 短信发送失败");
            return false;
        }
    }
    Serial.println("[Air780E] ❌ 短信发送超时");
    return false;
}

void Air780E_CheckTHAlert(float t, float h) {
    if (t > ALERT_TEMP_THRESH && h > ALERT_HUMI_THRESH) {
        consecAlertCount++;
        Serial.print("[Air780E] 温湿度超标计数: ");
        Serial.print(consecAlertCount);
        Serial.print("/"); Serial.println(ALERT_CONSEC_COUNT);

        if (consecAlertCount >= ALERT_CONSEC_COUNT && !thAlertSent) {
            char msg[80];
            sprintf(msg, "ALERT:High T&H! T=%.1fC H=%.1f%% Check crops now!", t, h);
            Air780E_SendSMS(msg);
            thAlertSent = true;
        }
    } else {
        if (consecAlertCount > 0) {
            Serial.println("[Air780E] 温湿度恢复正常，重置告警计数");
        }
        consecAlertCount = 0;
        thAlertSent = false;
    }
}

void Air780E_SendDDAlert(float dd) {
    char msg[60];
    sprintf(msg, "ALERT:DD=%.1f exceeded 120! Pest risk HIGH!", dd);
    Air780E_SendSMS(msg);
}