#include <Arduino.h>
#include "mod_dht22.h"

DHT dht(DHT_PIN, DHT_TYPE);

float gtemp = 0.0f;
float ghumi = 0.0f;

static uint8_t failCount = 0;

void DHT22_Init() {
    dht.begin();
    Serial.println("[DHT22] 初始化完成，引脚 PA0");
}

bool DHT22_Read() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
        gtemp = t;
        ghumi = h;
        failCount = 0;
        Serial.print("[DHT22] T=");
        Serial.print(t, 1);
        Serial.print("C  H=");
        Serial.print(h, 1);
        Serial.println("%");
        return true;
    }

    failCount++;
    Serial.print("[DHT22] 读取失败 #");
    Serial.println(failCount);
    if (failCount >= 3) {
        Serial.println("[DHT22] ERROR: 连续3次失败，检查PA0接线");
    }
    return false;
}