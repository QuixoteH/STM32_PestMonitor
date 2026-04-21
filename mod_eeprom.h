#ifndef MOD_EEPROM_H
#define MOD_EEPROM_H

#include <Arduino.h>
#include <Wire.h>

#define EEPROM_I2C_ADDR  0x50
#define ADDR_DD          0x0000   // float 4字节：累积积温
#define ADDR_FLAG        0x0004   // uint32 4字节：有效性魔数
#define ADDR_DAILY       0x0010   // float×7 = 28字节：7日均温历史
#define VALID_FLAG       0xDEADBEEF

void EEPROM_Init();
void EEPROM_WriteFloat(uint16_t addr, float val);
float EEPROM_ReadFloat(uint16_t addr);
void EEPROM_WriteUint32(uint16_t addr, uint32_t val);
uint32_t EEPROM_ReadUint32(uint16_t addr);
float EEPROM_LoadDD();
void EEPROM_SaveDD(float dd);
void EEPROM_Test();

#endif