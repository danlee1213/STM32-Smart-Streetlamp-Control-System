#ifndef __DHT11_H__
#define __DHT11_H__

#include "stm32f10x.h"

void DHT11_Init(void);
void DHT11_Start(void);

uint8_t DHT11_GetTemperature(void);
uint8_t DHT11_GetHumidity(void);
uint8_t DHT11_Check(void);

#endif
