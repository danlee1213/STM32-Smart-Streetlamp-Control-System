#include "delay.h"

static uint16_t fac_us = 0;	//us   
static uint16_t fac_ms = 0;	//ms


void Delay_Init(){
	
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); 
	fac_ms = fac_us * 1000;	
}

void delay_u(uint32_t time)
{
 register uint16_t i;

 for(i=0; i < time; i++){                                                                                                  //4clock delay
  __asm volatile("NOP"); __asm volatile("NOP"); __asm volatile("NOP"); __asm volatile("NOP");  //4clock delay
  __asm volatile("NOP"); __asm volatile("NOP"); __asm volatile("NOP"); __asm volatile("NOP");
  __asm volatile("NOP"); __asm volatile("NOP"); __asm volatile("NOP"); __asm volatile("NOP");
  __asm volatile("NOP"); __asm volatile("NOP"); __asm volatile("NOP"); __asm volatile("NOP");
  __asm volatile("NOP"); __asm volatile("NOP"); __asm volatile("NOP"); __asm volatile("NOP");   // 20clock delay
 }
} // 24clock delay  =  1usec delay

void delay_m(uint32_t time)
{
 register uint16_t i;
 for(i=0; i<time; i++) delay_u(1000);  // 1msec
}


/********************************* END OF FILE ********************************/
/******************************************************************************/
