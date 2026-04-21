#ifndef __ESP32_WIFI_HPP_
#define __ESP32_WIFI_HPP_

#include <stdio.h>
#include <string.h>
#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ESPWIFISerial  Serial

#define MODE_AP           0
#define MODE_STA          0
#define MODE_AP_STA       1

typedef struct ESP32_AI_Msg_t {
  int16_t lx;
  int16_t ly;
  int16_t rx;
  int16_t ry;
  int16_t cx;
  int16_t cy;
  uint16_t area;
  int16_t id;
} ESP32_AI_Msg;

typedef struct QR_AI_Msg_t {
  char QR_msg[50];
} QR_AI_Msg;

typedef enum AI_mode_t {
    Nornal_AI  = 0,
    Cat_Dog_AI = 1,
    FACE_AI    = 2,
    COLOR_AI   = 3,
    REFACE_AI  = 4,
    QR_AI      = 5,
    AI_MAX
} AI_mode;

void serial_init(void);
void SET_ESP_AI_MODE(AI_mode Mode);
void SET_ESP_WIFI_MODE(void);
void SET_STA_WIFI(void);
void SET_AP_WIFI(void);
void Get_STAIP(void);
void Get_APIP(void);
void Get_Version(void);
void recv_data(void);
void Data_Deal(char RXdata);
void recv_tcp_data(char tcpdata);

extern uint8_t newlines;
extern AI_mode runmode;
extern QR_AI_Msg QR_msg;
extern ESP32_AI_Msg esp32_ai_msg;
extern char data_buff[50];

#ifdef __cplusplus
}
#endif

#endif