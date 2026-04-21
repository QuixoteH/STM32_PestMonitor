#ifndef MOD_DD_CALC_H
#define MOD_DD_CALC_H

#include <Arduino.h>

#define DD_BASE_TEMP     10.0f
#define DD_ALERT_THRESH  120.0f
#define SAMPLE_INTERVAL  5000
#define DAY_MS           86400000UL   // 24小时

// EEPROM 地址（紧接 mod_eeprom 使用的地址之后）
#define ADDR_LAST_SETTLE_DAY  0x0008  // uint32 4字节，存已结算天数

extern float g_accumulated_dd;
extern uint32_t g_day_count;          // 开机后已过天数，供打印

void DD_Init(float saved_dd);
void DD_Update(float temp);
float DD_GetAccumulated();
void DD_ResetDaily();
void DD_PrintStatus();

#endif