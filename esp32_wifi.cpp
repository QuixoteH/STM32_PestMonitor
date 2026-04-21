#include "esp32_wifi.hpp"
#include <stdio.h>

#define IRMODELSerial Serial2

#if MODE_AP_STA
  #define WIFI_MODE '2'
#elif MODE_STA
  #define WIFI_MODE '1'
#elif MODE_AP
  #define WIFI_MODE '0'
#else
  #define WIFI_MODE '2'
#endif

#define APIP          "ap_ip"
#define AP_WIFI_SSID  "ESP32_WIFI_TEST"
#define AP_WIFI_PD    ""

#define STAIP         "sta_ip"
#define STA_WIFI_SSID "lbw"
#define STA_WIFI_PD   "lbwnblbwnb"

extern AI_mode runmode = Nornal_AI;
ESP32_AI_Msg esp32_ai_msg;
QR_AI_Msg QR_msg;

char send_buf[35]  = {0};
char recv_buf[50]  = {0};
char data_buff[50] = {0};
uint8_t cmd_flag   = 0;
uint8_t esp_time   = 0;

void serial_init(void) {
    ESPWIFISerial.begin(115200);
    Serial2.begin(115200);
}

void SET_STA_WIFI(void) {
    sprintf(send_buf, "sta_ssid:%s", STA_WIFI_SSID);
    IRMODELSerial.print(send_buf);
    memset(send_buf, 0, sizeof(send_buf));
    delay(300);
    sprintf(send_buf, "sta_pd:%s", STA_WIFI_PD);
    IRMODELSerial.print(send_buf);
    memset(send_buf, 0, sizeof(send_buf));
    delay(2000);
}

void SET_AP_WIFI(void) {
    sprintf(send_buf, "ap_ssid:%s", AP_WIFI_SSID);
    IRMODELSerial.print(send_buf);
    memset(send_buf, 0, sizeof(send_buf));
    delay(300);
    sprintf(send_buf, "ap_pd:%s", AP_WIFI_PD);
    IRMODELSerial.print(send_buf);
    memset(send_buf, 0, sizeof(send_buf));
    delay(2000);
}

void SET_ESP_WIFI_MODE(void) {
    sprintf(send_buf, "wifi_mode:%c", WIFI_MODE);
    IRMODELSerial.print(send_buf);
    memset(send_buf, 0, sizeof(send_buf));
    delay(2000);
}

void SET_ESP_AI_MODE(AI_mode Mode) {
    sprintf(send_buf, "ai_mode:%d", Mode);
    IRMODELSerial.print(send_buf);
    memset(send_buf, 0, sizeof(send_buf));
    runmode = Mode;
    delay(2000);
}

void Get_STAIP(void) {
    sprintf(send_buf, STAIP);
    IRMODELSerial.print(send_buf);
    memset(send_buf, 0, sizeof(send_buf));
    cmd_flag = 1;
}

void Get_APIP(void) {
    sprintf((char*)send_buf, APIP);
    IRMODELSerial.print(send_buf);
    memset(send_buf, 0, sizeof(send_buf));
    cmd_flag = 1;
}

void Get_Version(void) {
    sprintf((char*)send_buf, "wifi_ver");
    IRMODELSerial.print(send_buf);
    memset(send_buf, 0, sizeof(send_buf));
    cmd_flag = 1;
}

void recv_data(void) {
    char strr;
    if (IRMODELSerial.available()) {
        strr = char(IRMODELSerial.read());
        if (esp_time < 65) {
            ESPWIFISerial.print(strr);
            esp_time = esp_time + 1;
        }
        Data_Deal(strr);
    }
}

uint8_t end_falg = 0;
uint8_t i_index  = 0;

void Data_Deal(char RXdata) {
    if (cmd_flag == 1) {
        recv_buf[i_index] = RXdata;
        if (RXdata == 0x0D) end_falg = 1;
        if (end_falg == 1 && RXdata == 0x0A) {
            cmd_flag = 0;
            end_falg = 0;
            memcpy(data_buff, recv_buf, i_index);
            memset(recv_buf, 0, sizeof(recv_buf));
            i_index = 0;
        } else {
            i_index++;
        }
    } else if (cmd_flag == 2) {
        recv_tcp_data(RXdata);
    }
    // cmd_flag 3/4/5 对应AI模式，本项目不使用，不处理
}

uint8_t g_new_flag = 0;
uint8_t g_index    = 0;
uint8_t newlines   = 0;

void recv_tcp_data(char tcpdata) {
    if (tcpdata == '$' && g_new_flag == 0) {
        g_new_flag = 1;
        memset(recv_buf, 0, sizeof(recv_buf));
        return;
    }
    if (g_new_flag == 1) {
        if (tcpdata == '#') {
            g_new_flag = 0;
            g_index = 0;
            memcpy(data_buff, recv_buf, sizeof(recv_buf));
            memset(recv_buf, 0, sizeof(recv_buf));
        } else if (tcpdata == '$') {
            g_index = 0;
            g_new_flag = 0;
            memset(recv_buf, 0, sizeof(recv_buf));
        } else {
            recv_buf[g_index++] = tcpdata;
        }
        if (g_index > 50) {
            g_index = 0;
            g_new_flag = 0;
            memset(recv_buf, 0, sizeof(recv_buf));
        }
    }
}