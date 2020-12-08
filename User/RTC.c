#include "rtc.h"
#include "lcdtpv1.h"
#include "bsp_xpt2046_lcd.h"
#include "delay.h"
#include <stdio.h>

struct Data_Time
{
	uint8_t hour;
	uint8_t min;
	uint8_t sec;			

	uint16_t w_year;
	uint8_t  w_month;
	uint8_t  w_date;
	uint8_t  week;
};

struct Data_Time  timer;  
char time_clock[20];
char time_date[20];

uint8_t Init_RTC(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	#ifdef  VECT_TAB_RAM  								

	  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);  			
	#else  

	  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0); 	
	#endif
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);		

	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;		
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);		
	/*----------------------------------*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	
	PWR_BackupAccessCmd(ENABLE);									
	
	if(BKP_ReadBackupRegister(BKP_DR1)!=0x5555)					
	{	
     
		BKP_DeInit();											
		RCC_LSEConfig(RCC_LSE_ON);								
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)			
  		{}
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);					  
		RCC_RTCCLKCmd(ENABLE);									
		RTC_WaitForSynchro();								
		RTC_WaitForLastTask();									
		RTC_ITConfig(RTC_IT_SEC, ENABLE);						
		RTC_WaitForLastTask();										
		RTC_SetPrescaler(32767); 									
		RTC_WaitForLastTask();																					
    BKP_WriteBackupRegister(BKP_DR1, 0x5555);												
	}																 	
	else															
	{
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)		
		{
      	
		}											
		else if(RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)		
		{
      	
		}
    	
		
		RTC_WaitForSynchro();									
		
		RTC_ITConfig(RTC_IT_SEC, ENABLE);							

		RTC_WaitForLastTask();									
	}		    				     
	//Time_Get();														
	
	RCC_ClearFlag();											
	
	return 0; 	
}

uint8_t const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; 			  

const uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};	

uint8_t Time_Update(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint16_t t;
	uint32_t seccount=0;
	if(syear<1970||syear>2099)return 1;	   
	for(t=1970;t<syear;t++)											
		{
		if(Is_Leap_Year(t))seccount+=31622400;		
		else seccount+=31536000;			  				
		}
	smon-=1;
	for(t=0;t<smon;t++)	   											
		{
		seccount+=(uint32_t)mon_table[t]*86400;						
		if(Is_Leap_Year(syear)&&t==1)seccount+=86400;				  
		}
	seccount+=(uint32_t)(sday-1)*86400;							
	seccount+=(uint32_t)hour*3600;					
	seccount+=(uint32_t)min*60;	 					
	seccount+=sec;													

	RTC_WaitForLastTask();											
	RTC_SetCounter(seccount);									
	RTC_WaitForLastTask();											
	return 0;		
}

uint8_t Time_Get(void)
{
	static uint16_t daycnt=0;
	uint32_t timecount=0; 
	uint32_t temp=0;
	uint16_t temp1=0;	  
	 
	timecount = RTC_GetCounter();									
	temp=timecount/86400;   
	if(daycnt!=temp)
	{	  
		daycnt=temp;
		temp1=1970;	
		while(temp>=365)
		{				 
			if(Is_Leap_Year(temp1))
			{
				if(temp>=366)temp-=366;
				else {temp1++;break;}  
			}
			else temp-=365;	  
			temp1++;  
		}   
		timer.w_year=temp1;
		temp1=0;
		while(temp>=28)
		{
			if(Is_Leap_Year(timer.w_year)&&temp1==1)
			{
				if(temp>=29)temp-=29;
				else break; 
			}
			else 
			{
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];//??
				else break;
			}
			temp1++;  
		}
		timer.w_month=temp1+1;
		timer.w_date=temp+1;  
	}
	temp=timecount%86400;        	   
	timer.hour=temp/3600;  
	timer.min=(temp%3600)/60; 
	timer.sec=(temp%3600)%60; 
	timer.week=RTC_Get_Week(timer.w_year,timer.w_month,timer.w_date);
		sprintf(time_clock, "%0.2d%c%0.2d%c%0.2d",timer.hour , ':' , timer.min , ':'  , timer.sec);
		sprintf(time_date, "%0.4d%c%0.2d%c%0.2d",  timer.w_year , '/' , timer.w_month , '/' , timer.w_date);
	LCD_DrawString(115, 125, time_clock);
	LCD_DrawString(150,140, time_date);
	return 0;
}

uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day)
{	
	uint16_t temp2;
	uint8_t yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 	 
	if (yearH>19)yearL+=100;
  
	temp2=yearL+yearL/4; 
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
} 

uint8_t Is_Leap_Year(uint16_t year)
{			  
	if(year%4==0) 
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1; 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}

void RTC_IRQHandler(void)
{							    
	if(RTC_GetITStatus(RTC_IT_SEC))		
	{							
		//Time_Get();							
		
	}
	if(RTC_GetITStatus(RTC_IT_ALR))			
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);		  								 				
	}
	RTC_ClearITPendingBit(RTC_IT_SEC);				  								 
	RTC_WaitForLastTask();				
}

void Time_Set(uint16_t year , uint16_t month ,uint16_t day ,uint16_t hour ,uint16_t min ,uint16_t second )
{
	uint16_t Data[6];	
	Data[0] = year;
	Data[1] = month;
	Data[2] = day;
	Data[3] = hour;
	Data[4] = min;
	Data[5] = second;
	
  	Time_Update(Data[0],Data[1],Data[2],Data[3],Data[4],Data[5]);
}

uint8_t time_adjust(){
			strType_XPT2046_Coordinate strDisplayCoordinate;
			uint8_t currentHour = timer.hour;
			uint8_t currentMin = timer.min;
			uint8_t currentSec = timer.sec;
			char timeStore[20];
		
			LCD_GramScan ( 1 );
			LCD_Clear ( 0, 0, 240, 320, BACKGROUND );
			LCD_DrawString(65, 40, "Time adjustment");
			LCD_DrawString(20, 60, "Adjust your desired time");
			LCD_DrawString(20, 90, "Time : ");
			sprintf(timeStore, "%0.2d%c%0.2d%c%0.2d",timer.hour , ':' , timer.min , ':'  , timer.sec);
			LCD_DrawString(90, 90, timeStore);
			
			//Adjustment buttons
			LCD_DrawString(40, 130, "Hour +");
			LCD_DrawString(150, 130, "Hour -");
			LCD_DrawString(40, 180, "Minute +");
			LCD_DrawString(150, 180, "Minute -");
			LCD_DrawString(40, 230, "Second +");
			LCD_DrawString(150, 230, "Second -");
			LCD_DrawString(80, 280, "Set Time");

	while(1){
	//Touch Time +
				if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 45 ) && ( strDisplayCoordinate .y < 60 ) )
		{
			if ( ( strDisplayCoordinate .x > 175 ) && ( strDisplayCoordinate .x < 190 ) )
			{
				currentHour += 1;
				if(currentHour >= 24){
					currentHour = 24;
				}
				sprintf(timeStore, "%0.2d%c%0.2d%c%0.2d", currentHour , ':' , currentMin , ':'  , currentSec);
				LCD_DrawString(90, 90, timeStore);
				delay_m(300);
			}					
		}
	}
	
		//Touch Time -
				if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 165 ) && ( strDisplayCoordinate .y < 185 ) )
		{
			if ( ( strDisplayCoordinate .x > 175 ) && ( strDisplayCoordinate .x < 190 ) )
			{
				currentHour -= 1;
				if(currentHour == 255){
					currentHour = 0;
				}
				sprintf(timeStore, "%0.2d%c%0.2d%c%0.2d", currentHour , ':' , currentMin , ':'  , currentSec);
				LCD_DrawString(90, 90, timeStore);
				delay_m(300);
			}					
		}
	}
	
			//Touch Minute +
				if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 45 ) && ( strDisplayCoordinate .y < 80 ) )
		{
			if ( ( strDisplayCoordinate .x > 125 ) && ( strDisplayCoordinate .x < 140 ) )
			{
				currentMin += 1;
				if(currentMin >= 59){
					currentMin = 59;
				}
				sprintf(timeStore, "%0.2d%c%0.2d%c%0.2d", currentHour , ':' , currentMin , ':'  , currentSec);
				LCD_DrawString(90, 90, timeStore);
				delay_m(300);
			}					
		}
	}
	
		//Touch Minute -
				if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 155 ) && ( strDisplayCoordinate .y < 205 ) )
		{
			if ( ( strDisplayCoordinate .x > 125 ) && ( strDisplayCoordinate .x < 150 ) )
			{
				currentMin -= 1;
				
				if(currentMin == 255){
					currentMin = 0;
				}
				sprintf(timeStore, "%0.2d%c%0.2d%c%0.2d", currentHour , ':' , currentMin , ':'  , currentSec);
				LCD_DrawString(90, 90, timeStore);
				delay_m(300);
			}					
		}
	}
	
			//Touch Second +
				if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 40 ) && ( strDisplayCoordinate .y < 90 ) )
		{
			if ( ( strDisplayCoordinate .x > 75 ) && ( strDisplayCoordinate .x < 100 ) )
			{
				currentSec += 1;
				
				if(currentSec >= 59){
					currentSec = 59;
				}
				sprintf(timeStore, "%0.2d%c%0.2d%c%0.2d", currentHour , ':' , currentMin , ':'  , currentSec);
				LCD_DrawString(90, 90, timeStore);
				delay_m(300);
			}					
		}
	}
	
				//Touch Second -
				if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 170 ) && ( strDisplayCoordinate .y < 205 ) )
		{
			if ( ( strDisplayCoordinate .x > 70 ) && ( strDisplayCoordinate .x < 90 ) )
			{
				currentSec -= 1;
				
				if(currentSec == 255){
					currentSec = 0;
				}
				sprintf(timeStore, "%0.2d%c%0.2d%c%0.2d", currentHour , ':' , currentMin , ':'  , currentSec);
				LCD_DrawString(90, 90, timeStore);
				delay_m(300);
			}					
		}
	}
	
	
					//Set new time
				if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 103 ) && ( strDisplayCoordinate .y < 130 ) )
		{
			if ( ( strDisplayCoordinate .x > 35 ) && ( strDisplayCoordinate .x < 50 ) )
			{
				Time_Set(timer.w_year , timer.w_month ,timer.w_date ,currentHour ,currentMin ,currentSec );
				return function_flag = 2;
	
			}					
		}
	}
}
	return 0;
}

uint8_t date_adjust(){
			strType_XPT2046_Coordinate strDisplayCoordinate;
			
			uint8_t currentYear = timer.w_year;
			uint8_t currentMonth = timer.w_month;
			uint8_t currentDay = timer.w_date;
			char dateStore[20];

			//display_co();
			LCD_GramScan ( 1 );
			LCD_Clear ( 0, 0, 240, 320, BACKGROUND );
			LCD_DrawString(65, 40, "Date adjustment");
			LCD_DrawString(20, 60, "Adjust your desired date");
			LCD_DrawString(30, 90, "Date : ");
			sprintf(dateStore, "%0.4d%c%0.2d%c%0.2d",  timer.w_year , '/' , timer.w_month , '/' , timer.w_date);
			LCD_DrawString(90, 90, dateStore);
			
			//Adjustment buttons
			LCD_DrawString(40, 130, "Year +");
			LCD_DrawString(150, 130, "Year -");
			LCD_DrawString(40, 180, "Month +");
			LCD_DrawString(150, 180, "Month -");
			LCD_DrawString(40, 230, "Day +");
			LCD_DrawString(150, 230, "Day -");
			LCD_DrawString(80, 280, "Set Date");
	
	while(1){
	//Touch Year +
				if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 45 ) && ( strDisplayCoordinate .y < 60 ) )
		{
			if ( ( strDisplayCoordinate .x > 175 ) && ( strDisplayCoordinate .x < 190 ) )
			{
				currentYear += 1;
				sprintf(dateStore, "%0.4d%c%0.2d%c%0.2d",  currentYear , '/' , currentMonth , '/' , currentDay);
				LCD_DrawString(90, 90, dateStore);
				delay_m(300);
			}					
		}
	}
	
		//Touch Year -
				if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 165 ) && ( strDisplayCoordinate .y < 185 ) )
		{
			if ( ( strDisplayCoordinate .x > 175 ) && ( strDisplayCoordinate .x < 190 ) )
			{
				currentYear -= 1;
				sprintf(dateStore, "%0.4d%c%0.2d%c%0.2d",  currentYear , '/' , currentMonth , '/' , currentDay);
				LCD_DrawString(90, 90, dateStore);
				delay_m(300);
			}					
		}
	}
	
			//Touch Month +
				if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 45 ) && ( strDisplayCoordinate .y < 80 ) )
		{
			if ( ( strDisplayCoordinate .x > 125 ) && ( strDisplayCoordinate .x < 140 ) )
			{
				currentMonth += 1;
				if(currentMonth >= 12){
					currentMonth = 12;
				}
				sprintf(dateStore, "%0.4d%c%0.2d%c%0.2d",  currentYear , '/' , currentMonth , '/' , currentDay);
				LCD_DrawString(90, 90, dateStore);
				delay_m(300);
			}					
		}
	}
	
		//Touch Month -
				if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 155 ) && ( strDisplayCoordinate .y < 205 ) )
		{
			if ( ( strDisplayCoordinate .x > 125 ) && ( strDisplayCoordinate .x < 150 ) )
			{
				currentMonth -= 1;
				
				if(currentMonth == 0){
					currentMonth = 1;
				}
				sprintf(dateStore, "%0.4d%c%0.2d%c%0.2d",  currentYear , '/' , currentMonth , '/' , currentDay);
				LCD_DrawString(90, 90, dateStore);
				delay_m(300);
			}					
		}
	}
	
			//Touch Day +
				if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 40 ) && ( strDisplayCoordinate .y < 90 ) )
		{
			if ( ( strDisplayCoordinate .x > 75 ) && ( strDisplayCoordinate .x < 100 ) )
			{
				currentDay += 1;
				
				if(currentDay >= 31){
					currentDay = 31;
				}
				sprintf(dateStore, "%0.4d%c%0.2d%c%0.2d",  currentYear , '/' , currentMonth , '/' , currentDay);
				LCD_DrawString(90, 90, dateStore);
				delay_m(300);
			}					
		}
	}
	
				//Touch Day -
				if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 170 ) && ( strDisplayCoordinate .y < 205 ) )
		{
			if ( ( strDisplayCoordinate .x > 70 ) && ( strDisplayCoordinate .x < 90 ) )
			{
				currentDay -= 1;
				
				if(currentDay == 0){
					currentDay = 1;
				}
				sprintf(dateStore, "%0.4d%c%0.2d%c%0.2d",  currentYear , '/' , currentMonth , '/' , currentDay);
				LCD_DrawString(90, 90, dateStore);
				delay_m(300);
			}					
		}
	}
	
	
					//Set new date
				if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 103 ) && ( strDisplayCoordinate .y < 130 ) )
		{
			if ( ( strDisplayCoordinate .x > 35 ) && ( strDisplayCoordinate .x < 50 ) )
			{
				timer.week=RTC_Get_Week(currentYear,currentMonth,currentDay);
				Time_Set(currentYear , currentMonth ,currentDay ,timer.hour ,timer.min ,timer.sec );
				return function_flag = 2;
	
			}					
		}
	}
}
	return 0;
}

