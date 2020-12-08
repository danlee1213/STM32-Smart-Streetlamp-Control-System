#ifndef __BSP_XPT2046_LCD_H
#define	__BSP_XPT2046_LCD_H


#include "stm32f10x.h"


#define             macXPT2046_EXTI_GPIO_CLK                        RCC_APB2Periph_GPIOE     
#define             macXPT2046_EXTI_GPIO_PORT                       GPIOE   
#define             macXPT2046_EXTI_GPIO_PIN                        GPIO_Pin_4
#define             macXPT2046_EXTI_SOURCE_PORT                     GPIO_PortSourceGPIOE
#define             macXPT2046_EXTI_SOURCE_PIN                      GPIO_PinSource4
#define             macXPT2046_EXTI_LINE                            EXTI_Line4
#define             macXPT2046_EXTI_IRQ                             EXTI4_IRQn
#define             macXPT2046_EXTI_INT_FUNCTION                    EXTI4_IRQHandler

#define             macXPT2046_EXTI_ActiveLevel                     0
#define             macXPT2046_EXTI_Read()                          GPIO_ReadInputDataBit ( macXPT2046_EXTI_GPIO_PORT, macXPT2046_EXTI_GPIO_PIN )


#define             macXPT2046_SPI_GPIO_CLK                         RCC_APB2Periph_GPIOE| RCC_APB2Periph_GPIOD

#define             macXPT2046_SPI_CS_PIN		            				GPIO_Pin_13
#define             macXPT2046_SPI_CS_PORT		            			GPIOD

#define	            macXPT2046_SPI_CLK_PIN	                    GPIO_Pin_0
#define             macXPT2046_SPI_CLK_PORT	                    GPIOE

#define	            macXPT2046_SPI_MOSI_PIN	                    GPIO_Pin_2
#define	            macXPT2046_SPI_MOSI_PORT	                  GPIOE

#define	            macXPT2046_SPI_MISO_PIN	                    GPIO_Pin_3
#define	            macXPT2046_SPI_MISO_PORT	                  GPIOE


#define             macXPT2046_CS_ENABLE()                          GPIO_SetBits ( macXPT2046_SPI_CS_PORT, macXPT2046_SPI_CS_PIN )    
#define             macXPT2046_CS_DISABLE()                         GPIO_ResetBits ( macXPT2046_SPI_CS_PORT, macXPT2046_SPI_CS_PIN )  

#define             macXPT2046_CLK_HIGH()                           GPIO_SetBits ( macXPT2046_SPI_CLK_PORT, macXPT2046_SPI_CLK_PIN )    
#define             macXPT2046_CLK_LOW()                            GPIO_ResetBits ( macXPT2046_SPI_CLK_PORT, macXPT2046_SPI_CLK_PIN ) 

#define             macXPT2046_MOSI_1()                             GPIO_SetBits ( macXPT2046_SPI_MOSI_PORT, macXPT2046_SPI_MOSI_PIN ) 
#define             macXPT2046_MOSI_0()                             GPIO_ResetBits ( macXPT2046_SPI_MOSI_PORT, macXPT2046_SPI_MOSI_PIN )

#define             macXPT2046_MISO()                               GPIO_ReadInputDataBit ( macXPT2046_SPI_MISO_PORT, macXPT2046_SPI_MISO_PIN )


#define             macXPT2046_Coordinate_GramScan                  1               
#define             macXPT2046_THRESHOLD_CalDiff                    2              

#define	            macXPT2046_CHANNEL_X 	                          0x90 	         
#define	            macXPT2046_CHANNEL_Y 	                          0xd0	        



typedef	struct         
{	
   uint16_t x;		
   uint16_t y;
	
} strType_XPT2046_Coordinate;   


typedef struct        
{
	long double An,  		 
              Bn,     
              Cn,   
              Dn,    
              En,    
              Fn,     
              Divider;
	
} strType_XPT2046_Calibration;


typedef struct         
{
	long double dX_X,  			 
              dX_Y,     
              dX,   
              dY_X,    
              dY_Y,    
              dY;

} strType_XPT2046_TouchPara;



extern volatile uint8_t               ucXPT2046_TouchFlag;

extern strType_XPT2046_TouchPara      strXPT2046_TouchPara;

extern volatile int function_flag;

extern volatile uint32_t ADCConvertedValue1;
extern volatile uint32_t ADCConvertedValue2;


void                     XPT2046_Init                    ( void );

uint8_t                  XPT2046_Touch_Calibrate         ( void );
uint8_t                  XPT2046_Get_TouchedPoint        ( strType_XPT2046_Coordinate * displayPtr, strType_XPT2046_TouchPara * para );
 
//void delay_m(uint32_t time);
void display_co(void);
void Check_touchkey (void); 
void Next_touchkey(void);
void Menu_conf(void);
void Menu_touch(void);
void Clock_config(void);
void auto_light_mode(void);
void manual_control_mode(void);
void dht11_display(void);
//-----------------------------
  

#endif /* __BSP_TOUCH_H */

