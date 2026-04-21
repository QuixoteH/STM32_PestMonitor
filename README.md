# 农作物害虫监测系统下位机

## 简介

基于 STM32F103C8T6的农业害虫监测下位机固件，实现环境温湿度采集、积温累积计算、视频推流配置与短信告警，配合上位机识别软件使用。

## 程序架构

主循环模型：
setup() → 外设初始化、EEPROM恢复积温、ESP32-CAM配置、告警模块注册入网
loop() → 5秒温湿度采样、实时告警判断、24h积温日结算

模块划分：
esp32_wifi ESP32-CAM WiFi配置 / 视频推流 / 串口协议解析（由摄像头厂家提供的代码修改适配）
mod_dht22 DHT22温湿度采集
mod_eeprom AT24C256 I2C读写 / 积温断电持久化
mod_air780e  告警模块AT指令驱动 / 启动短信 / 温湿度和积温告警
mod_dd_calc 有效积温计算 / 24h日结算 / 告警阈值判断


## 硬件平台

| 器件 | 功能 | 接口 |
|------|------|------|
| STM32F103C8T6 | 主控 | — |
| Yahboom ESP32-CAM Lite | 视频推流| USART2 PA2/PA3 |
| DHT22 | 温湿度采集 | GPIO PA0 |
| AT24C256 | 积温持久化存储 | I2C PB6/PB7 |
| air780e  | GSM短信告警（移动卡）| USART1 PA9/PA10 |


## 技术栈与工具

| 类别 | 选型 |
|------|------|
| 语言 | C++（Arduino框架）|
| 开发环境 | Arduino IDE  |
| 烧录工具 | ST-Link V2 |
| 串口调试 | Uartassist |
| AI 代码辅助 | Claude Sonnet 4.6 Thinking |

