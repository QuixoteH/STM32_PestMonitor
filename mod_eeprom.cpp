#include <Arduino.h>
#include "mod_eeprom.h"

void EEPROM_Init() {
    Wire.begin();
    Serial.println("[EEPROM] 初始化完成，I2C PB6(SCL)/PB7(SDA)");
}

void EEPROM_WriteFloat(uint16_t addr, float val) {
    byte *p = (byte*)&val;
    Wire.beginTransmission(EEPROM_I2C_ADDR);
    Wire.write((addr >> 8) & 0xFF);
    Wire.write(addr & 0xFF);
    for (int i = 0; i < 4; i++) Wire.write(p[i]);
    Wire.endTransmission();
    delay(5);
}

float EEPROM_ReadFloat(uint16_t addr) {
    float val = 0.0f;
    byte *p = (byte*)&val;
    Wire.beginTransmission(EEPROM_I2C_ADDR);
    Wire.write((addr >> 8) & 0xFF);
    Wire.write(addr & 0xFF);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)EEPROM_I2C_ADDR, (uint8_t)4);
    for (int i = 0; i < 4; i++) {
        if (Wire.available()) p[i] = Wire.read();
    }
    return val;
}

void EEPROM_WriteUint32(uint16_t addr, uint32_t val) {
    byte *p = (byte*)&val;
    Wire.beginTransmission(EEPROM_I2C_ADDR);
    Wire.write((addr >> 8) & 0xFF);
    Wire.write(addr & 0xFF);
    for (int i = 0; i < 4; i++) Wire.write(p[i]);
    Wire.endTransmission();
    delay(5);
}

uint32_t EEPROM_ReadUint32(uint16_t addr) {
    uint32_t val = 0;
    byte *p = (byte*)&val;
    Wire.beginTransmission(EEPROM_I2C_ADDR);
    Wire.write((addr >> 8) & 0xFF);
    Wire.write(addr & 0xFF);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)EEPROM_I2C_ADDR, (uint8_t)4);
    for (int i = 0; i < 4; i++) {
        if (Wire.available()) p[i] = Wire.read();
    }
    return val;
}

float EEPROM_LoadDD() {
    uint32_t flag = EEPROM_ReadUint32(ADDR_FLAG);
    if (flag == VALID_FLAG) {
        float dd = EEPROM_ReadFloat(ADDR_DD);
        Serial.print("[EEPROM] 恢复积温: ");
        Serial.println(dd);
        return dd;
    }
    Serial.println("[EEPROM] 首次启动，积温从0开始");
    return 0.0f;
}

void EEPROM_SaveDD(float dd) {
    EEPROM_WriteFloat(ADDR_DD, dd);
    EEPROM_WriteUint32(ADDR_FLAG, VALID_FLAG);
    Serial.print("[EEPROM] 已保存积温: ");
    Serial.println(dd);
}

// 专用测试函数：写入测试值，断电重启后验证是否一致
void EEPROM_Test() {
    Serial.println("[EEPROM] ---- 开始测试 ----");

    float writeVal = 88.5f;
    Serial.print("[EEPROM] 写入测试值: ");
    Serial.println(writeVal);
    EEPROM_WriteFloat(ADDR_DD, writeVal);
    EEPROM_WriteUint32(ADDR_FLAG, VALID_FLAG);

    delay(10);

    float readVal = EEPROM_ReadFloat(ADDR_DD);
    uint32_t flag  = EEPROM_ReadUint32(ADDR_FLAG);

    Serial.print("[EEPROM] 读回数值: ");
    Serial.println(readVal);
    Serial.print("[EEPROM] 魔数标志: 0x");
    Serial.println(flag, HEX);

    if (readVal > 88.4f && readVal < 88.6f && flag == VALID_FLAG) {
        Serial.println("[EEPROM] ✅ 读写测试通过");
    } else {
        Serial.println("[EEPROM] ❌ 读写测试失败，检查PB6/PB7接线");
    }

    Serial.println("[EEPROM] ---- 测试结束 ----");
    Serial.println("[EEPROM] 请断电重启，验证数据是否持久化");
}