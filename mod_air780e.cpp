#include <Arduino.h>
#include "mod_air780e.h"
#include "mod_dd_calc.h"

#define DBG Serial1

static bool    moduleReady       = false;
static uint8_t consecAlertCount  = 0;
static bool    thAlertSent       = false;

static bool sendAT(const char* cmd, const char* expect, uint32_t timeout = 5000) {
    while (AIR780E_SERIAL.available()) AIR780E_SERIAL.read();

    // 发送指令 + \r\n（关键修复：Air780E需要\r\n才能识别指令）
    AIR780E_SERIAL.print(cmd);
    AIR780E_SERIAL.print("\r\n");

    DBG.print("[AT>>] "); DBG.println(cmd);

    String resp = "";
    uint32_t t = millis();
    while (millis() - t < timeout) {
        while (AIR780E_SERIAL.available()) {
            resp += (char)AIR780E_SERIAL.read();
        }
        if (resp.indexOf(expect) >= 0) {
            DBG.print("[AT<<] "); DBG.println(resp);
            return true;
        }
        if (resp.indexOf("ERROR") >= 0) {
            DBG.print("[AT<<ERR] "); DBG.println(resp);
            return false;
        }
    }
    DBG.print("[AT<<TIMEOUT] "); DBG.println(resp);
    return false;
}

bool Air780E_Init() {
    AIR780E_SERIAL.begin(AIR780E_BAUDRATE);
    DBG.println("[Air780E] 模块上电，等待就绪...");
    delay(8000);

    bool alive = false;
    for (int i = 0; i < 10; i++) {
        if (sendAT("AT", "OK", 2000)) { alive = true; break; }
        DBG.print("[Air780E] 握手重试 "); DBG.println(i + 1);
        delay(2000);
    }
    if (!alive) {
        DBG.println("[Air780E] ❌ 无响应，检查PA2/PA3接线和供电");
        return false;
    }
    DBG.println("[Air780E] AT 通信正常");

    sendAT("ATE0", "OK", 2000);

    if (!sendAT("AT+CPIN?", "READY", 5000)) {
        DBG.println("[Air780E] ❌ SIM卡未就绪");
        return false;
    }
    DBG.println("[Air780E] SIM卡就绪");

    DBG.println("[Air780E] 正在注册网络...");
    bool netOk = false;
    for (int i = 0; i < 15; i++) {
        AIR780E_SERIAL.print("AT+CREG?\r\n");
        delay(2000);
        String r = "";
        uint32_t t = millis();
        while (millis() - t < 1000) {
            while (AIR780E_SERIAL.available()) r += (char)AIR780E_SERIAL.read();
        }
        DBG.print("[CREG] "); DBG.println(r);
        if (r.indexOf(",1") >= 0 || r.indexOf(",5") >= 0) {
            netOk = true;
            DBG.println("[Air780E] 网络注册成功");
            break;
        }
        DBG.print("[Air780E] 等待网络 "); DBG.print(i + 1); DBG.println("/15");
    }
    if (!netOk) {
        DBG.println("[Air780E] ❌ 网络注册失败");
        return false;
    }

    if (!sendAT("AT+CMGF=1", "OK", 3000)) return false;
    if (!sendAT("AT+CSCS=\"GSM\"", "OK", 3000)) return false;

    moduleReady = true;
    DBG.println("[Air780E] ✅ 初始化完成");
    Air780E_SendSMS("PestMonitor system started.");
    return true;
}

bool Air780E_SendSMS(const char* content) {
    if (!moduleReady) {
        DBG.println("[Air780E] 模块未就绪，跳过短信");
        return false;
    }

    char cmd[40];
    sprintf(cmd, "AT+CMGS=\"%s\"", SMS_TARGET);

    if (!sendAT("AT+CMGF=1", "OK", 2000)) return false;
    if (!sendAT(cmd, ">", 8000)) {
        DBG.println("[Air780E] ❌ 未收到>提示符");
        return false;
    }

    AIR780E_SERIAL.print(content);
    AIR780E_SERIAL.write(0x1A);   // Ctrl+Z 结束短信
    DBG.print("[Air780E] 发送内容: "); DBG.println(content);

    String resp = "";
    uint32_t t = millis();
    while (millis() - t < 15000) {
        while (AIR780E_SERIAL.available()) resp += (char)AIR780E_SERIAL.read();
        if (resp.indexOf("+CMGS:") >= 0) {
            DBG.println("[Air780E] ✅ 短信发送成功");
            return true;
        }
        if (resp.indexOf("ERROR") >= 0) {
            DBG.println("[Air780E] ❌ 短信发送失败");
            return false;
        }
    }
    DBG.println("[Air780E] ❌ 短信发送超时");
    return false;
}

void Air780E_CheckTHAlert(float t, float h) {
    if (t > ALERT_TEMP_THRESH && h > ALERT_HUMI_THRESH) {
        consecAlertCount++;
        DBG.print("[Air780E] 温湿度超标计数: ");
        DBG.print(consecAlertCount); DBG.print("/"); DBG.println(ALERT_CONSEC_COUNT);

        if (consecAlertCount >= ALERT_CONSEC_COUNT && !thAlertSent) {
            char msg[80];
            sprintf(msg, "ALERT:High T&H! T=%.1fC H=%.1f%% Check crops now!", t, h);
            Air780E_SendSMS(msg);
            thAlertSent = true;
        }
    } else {
        if (consecAlertCount > 0) DBG.println("[Air780E] 温湿度恢复正常，重置计数");
        consecAlertCount = 0;
        thAlertSent = false;
    }
}

void Air780E_SendDDAlert(float dd) {
    char msg[60];
    sprintf(msg, "ALERT:DD=%.1f exceeded %.1f! Pest risk HIGH!", dd, (float)DD_ALERT_THRESH);
    Air780E_SendSMS(msg);
}
