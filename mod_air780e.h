#ifndef MOD_AIR780E_H
#define MOD_AIR780E_H

#include <Arduino.h>

// Serial1 = USART1: PA9(TX) → Air780E RX, PA10(RX) ← Air780E TX
#define AIR780E_SERIAL   Serial1
#define AIR780E_BAUDRATE 115200
#define SMS_TARGET       "13019091653"

// 告警阈值
#define ALERT_TEMP_THRESH  28.0f
#define ALERT_HUMI_THRESH  80.0f
#define ALERT_CONSEC_COUNT 3

bool Air780E_Init();
bool Air780E_SendSMS(const char* content);
void Air780E_CheckTHAlert(float t, float h);
void Air780E_SendDDAlert(float dd);

#endif