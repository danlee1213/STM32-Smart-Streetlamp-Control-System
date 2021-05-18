# STM32-Smart-Streetlamp-Control-System
Using STM32103FVE to build the Smart Streetlamp Control System

In order to emphasize its "ecofriendly", STM32 board will only be operated by the external solar power energy circuit.

__Using STM32 Standard Peripheral Libraries__

# Overview
<img src="https://user-images.githubusercontent.com/72503871/118680219-221a7680-b831-11eb-83d1-ce0b11f358fd.jpg" width="600">
This project aims to build the Smart Streetlamp Control System using solar power system.

STM32 Cortex-M3 provides following functions:
1. Touchscreen function using TFT LCD
2. Real Time Digital Clock
3. Auto-light control mode
4. Manual ligt control mode
5. Temperature & Humidity measurement

## Solar Power Energy Circuit Schematic
<div>
  <img src = https://user-images.githubusercontent.com/72503871/101485489-31e68280-3996-11eb-8bcf-51b2a8b7ade8.png width="430">
  <img src = https://user-images.githubusercontent.com/72503871/101485306-efbd4100-3995-11eb-82b1-7d0dcb99adf8.png>
</div>

The cirucit is designed to do following functions:
* Providing adequate power to operate STM32 board using solar power
* Solar power harvesting system
* Storing energy in rechargeable lithium batteries to opearte the board for a long time
* Preventing from excessive charging to ensure battery safety

2.4V Zener Diode is connected between the positive terminal of the solar panel and the emitter of the BJT.
It prevents from excessive charging as it stops current flow if the voltage provided by solar panel exceeds 2.4V, which could damage batteries.
LED indicates whether the battery is being charged by the solar panel or being consumed by STM32 board.

LED is connected to the collector of the transistor to show the charging state.
If voltage is applied by the solar panel, the light will be turned off.
If the light intensity is not enough to charge, then the light will be turned one due to the nature of the PNP transistor.

## TFT LCD Touchscreen

<img src = "https://user-images.githubusercontent.com/72503871/101482298-7ae80800-3991-11eb-869c-d2be5261883a.png" width="600" height="500">

Using 3.2” TFT LCD Display. Built-in XPT2046 chip can trigger the touchscreen function.

It is important to set the SPI communication to initialize SPI_MISO, SPI_MOSI, SPI_CLK, and SPI_CS_PIN for interrupt

Following code is used to configure SPI communication between XPT2046 and TFT LCD display:

```C
static void XPT2046_GPIO_SPI_Config (void) 
{ 
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd ( macXPT2046_SPI_GPIO_CLK, ENABLE );
       
  GPIO_InitStructure.GPIO_Pin=macXPT2046_SPI_CLK_PIN;
  GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz ;	  
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
  GPIO_Init(macXPT2046_SPI_CLK_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = macXPT2046_SPI_MOSI_PIN;
  GPIO_Init(macXPT2046_SPI_MOSI_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = macXPT2046_SPI_MISO_PIN; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;      
  GPIO_Init(macXPT2046_SPI_MISO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = macXPT2046_SPI_CS_PIN; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      
  GPIO_Init(macXPT2046_SPI_CS_PORT, &GPIO_InitStructure); 
   
  macXPT2046_CS_DISABLE();

}
```

External interrupt has to be initialized for the touchscreen function as well.

First, initialize the config of NVIC:

```C
static void XPT2046_EXTI_NVIC_Config (void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
 
  NVIC_InitStructure.NVIC_IRQChannel = macXPT2046_EXTI_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
}
```

Then, initialize the external interrupt:
```C
static void XPT2046_EXTI_Config (void)
{
  GPIO_InitTypeDef GPIO_InitStructure; 
  EXTI_InitTypeDef EXTI_InitStructure;
  
  /* config the extiline clock and AFIO clock */
  RCC_APB2PeriphClockCmd ( macXPT2046_EXTI_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE );
												
  /* config the NVIC */
  XPT2046_EXTI_NVIC_Config ();

  /* EXTI line gpio config*/	
  GPIO_InitStructure.GPIO_Pin = macXPT2046_EXTI_GPIO_PIN;       
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	 
  GPIO_Init(macXPT2046_EXTI_GPIO_PORT, &GPIO_InitStructure);

  /* EXTI line mode config */
  GPIO_EXTILineConfig(macXPT2046_EXTI_SOURCE_PORT, macXPT2046_EXTI_SOURCE_PIN); 
  EXTI_InitStructure.EXTI_Line = macXPT2046_EXTI_LINE;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	
  EXTI_Init(&EXTI_InitStructure); 	
}
```

## Ditigal Clock
STM32 provides built-in RTC (Real Time Clock) function.

First, initialize RTC:
```C
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
```


## Auto-light control
Using LDR and ADC to digitize analog signal from LDR

## Manual light control
PWM signal modulation

## Temperature & Humidity
Using DHT11 sensor and activate USART for serial communication


