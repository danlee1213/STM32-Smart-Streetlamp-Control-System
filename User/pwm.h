#ifndef __PWM_H
#define __PWM_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f10x.h"
	 
 
void PWM_INIT(void);
void led_dimming(void);
	 
#ifdef __cplusplus
}
#endif

#endif

