#include "dht11.h"
#include "delay.h"
#include <stdio.h>

#define DHT11_DATA_PIN 		GPIO_Pin_4
#define DHT11_DATA_GPIO 	GPIOA
#define DHT11_DATA_Periph	RCC_APB2Periph_GPIOA

static GPIO_InitTypeDef GPIO_InitStruct;
static uint8_t DHT11_data[5];

static void DHT11_SetInput(void);
static void DHT11_SetOutput(void);

static void DHT11_RecvData(void);
static uint8_t DHT11_RecvDataByte(void);
static uint8_t DHT11_RecvDataBit(void);

void DHT11_Init(void)
{
	
	RCC_APB2PeriphClockCmd(DHT11_DATA_Periph, ENABLE);
	
	delay_m(1000); // Delay_s(1)
}

void DHT11_Start(void)
{	
	DHT11_SetInput();

	GPIO_SetBits(DHT11_DATA_GPIO, DHT11_DATA_PIN); 
	delay_m(10); //Delay_ms(10)
	GPIO_ResetBits(DHT11_DATA_GPIO, DHT11_DATA_PIN);
	delay_m(20); //Delay_ms(20)
	GPIO_SetBits(DHT11_DATA_GPIO, DHT11_DATA_PIN); 
	delay_u(30); //Delay_us(30)
	
	DHT11_SetOutput();
	
	while (GPIO_ReadInputDataBit(DHT11_DATA_GPIO, DHT11_DATA_PIN) == SET);
	while (GPIO_ReadInputDataBit(DHT11_DATA_GPIO, DHT11_DATA_PIN) == RESET);
	while (GPIO_ReadInputDataBit(DHT11_DATA_GPIO, DHT11_DATA_PIN) == SET);


	DHT11_RecvData();
}

void DHT11_SetInput(void)
{

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = DHT11_DATA_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DHT11_DATA_GPIO, &GPIO_InitStruct);
}

void DHT11_SetOutput(void)
{

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin = DHT11_DATA_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DHT11_DATA_GPIO, &GPIO_InitStruct);
}

uint8_t DHT11_RecvDataBit(void)
{

	while (GPIO_ReadInputDataBit(DHT11_DATA_GPIO, DHT11_DATA_PIN) == RESET);
	
	
	delay_u(40); // Delay_us(40)
	if (GPIO_ReadInputDataBit(DHT11_DATA_GPIO, DHT11_DATA_PIN) == RESET)
	{
		return 0;
	}
	while (GPIO_ReadInputDataBit(DHT11_DATA_GPIO, DHT11_DATA_PIN) == SET);
	return 1;	
}

uint8_t DHT11_RecvDataByte(void)
{
	uint8_t data = 0; 
	uint8_t i = 0;
	for (i = 0; i < 8; ++i)
	{
		data <<= 1;
		data |= DHT11_RecvDataBit();
	}
	return data;
}

void DHT11_RecvData(void)
{
	uint8_t i = 0;
	for (i = 0; i < 5; ++i)
	{
		DHT11_data[i] = DHT11_RecvDataByte();
	}
}

uint8_t DHT11_GetTemperature(void)
{
	return DHT11_data[2];
}

uint8_t DHT11_GetHumidity(void)
{
	return DHT11_data[0];
}

uint8_t DHT11_Check(void)
{
	return (DHT11_data[0] + DHT11_data[1] + DHT11_data[2] + DHT11_data[3] 
				== DHT11_data[4]);
}

