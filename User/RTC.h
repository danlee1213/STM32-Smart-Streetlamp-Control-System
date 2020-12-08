#ifndef  __RTC_H
#define	 __RTC_H

#include "stm32f10x.h"

uint8_t Init_RTC(void);
uint8_t Is_Leap_Year(uint16_t year);
uint8_t Time_Update(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec);
uint8_t Time_Get(void);
uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day);
uint8_t time_adjust(void);
uint8_t date_adjust(void);

void Time_Set(uint16_t year , uint16_t month , uint16_t day , uint16_t hour , uint16_t min , uint16_t second);

#endif
