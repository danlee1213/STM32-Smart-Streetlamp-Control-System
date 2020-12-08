#include "stm32f10x.h"
#include "bsp_xpt2046_lcd.h"
#include "lcdtpv1.h"
#include "adc.h"
#include "pwm.h"
#include "delay.h"
#include "dht11.h"
#include "RTC.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>


GPIO_InitTypeDef GPIO_InitStructure;
void Delay1 ( __IO uint32_t nCount ){  for ( ; nCount != 0; nCount -- );}	

	
	char* pStr0 = 0;
	char * pStr1 = 0;
	char* pStr2 = 0;
	char* pStr3 = 0;


int main ( void )
{	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	LCD_INIT ();	
	XPT2046_Init ();
	init_adc1();
	init_adc2();
	PWM_INIT();
	Init_RTC();
	Time_Set(2020, 12, 4, 16, 45, 00);
	DHT11_Init();

	
	//Initial screen settings
	LCD_GramScan ( 1 );
	LCD_Clear ( 0, 0, 240, 320, YELLOW);
	pStr0 = "ACCESS";
	LCD_DrawString_Color(95, 280, pStr0, CYAN, BLACK);
	pStr1 = "Welcome!";
	LCD_DrawString_Color(90, 20, pStr1, YELLOW, BLACK);
	pStr2 = "Smart Streetlamp Control";
	LCD_DrawString_Color(20, 40, pStr2, YELLOW, BLACK);
	pStr3 = "System";
	LCD_DrawString_Color(90, 60, pStr3, YELLOW, BLACK);
	
	
	while ( 1 )
  {
    if ( ucXPT2046_TouchFlag == 1 )	         
    {
			Next_touchkey();
			
			if(function_flag == 1){
				
				Menu_conf();
				Menu_touch();
			}
			
			if(function_flag == 2){
				Clock_config();
			}
			
			if(function_flag == 3){			
				
				auto_light_mode();
			}
			
			if(function_flag == 4){
				
				 manual_control_mode();
			}
			
			if(function_flag == 5){
				dht11_display();
			}
			
			if(function_flag == 10){
				time_adjust();
			}
			
			if(function_flag == 11){
				date_adjust();
			}
			
      ucXPT2046_TouchFlag = 0;		    
			
    }	
		
	
		Delay1(500000);		
		
	}
  
}





/* ------------------------------------------end of file---------------------------------------- */

