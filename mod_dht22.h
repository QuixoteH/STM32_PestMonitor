#ifndef MOD_DHT22_H
#define MOD_DHT22_H

#include <Arduino.h>
#include <DHT.h>

#define DHT_PIN  PA0
#define DHT_TYPE DHT22

extern float gtemp;
extern float ghumi;

void DHT22_Init();
bool DHT22_Read();

#endif