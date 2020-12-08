#include "bsp_xpt2046_lcd.h"
#include "adc.h"
#include "delay.h"
#include "lcdtpv1.h"
#include "RTC.h"
#include "dht11.h"
#include <stdio.h> 
#include <string.h>
#include <time.h>
#include <stdlib.h>



static void                   XPT2046_EXTI_Config                   ( void );
static void                   XPT2046_EXTI_NVIC_Config              ( void );
static void                   XPT2046_GPIO_SPI_Config               ( void );

static void                   XPT2046_DelayUS                       ( __IO uint32_t ulCount );
static void                   XPT2046_WriteCMD                      ( uint8_t ucCmd );
static uint16_t               XPT2046_ReadCMD                       ( void );
static uint16_t               XPT2046_ReadAdc                       ( uint8_t ucChannel );
static void                   XPT2046_ReadAdc_XY                    ( int16_t * sX_Ad, int16_t * sY_Ad );
static uint8_t                XPT2046_ReadAdc_Smooth_XY             ( strType_XPT2046_Coordinate * pScreenCoordinate );
static uint8_t                XPT2046_Calculate_CalibrationFactor   ( strType_XPT2046_Coordinate * pDisplayCoordinate, strType_XPT2046_Coordinate * pScreenSample, strType_XPT2046_Calibration * pCalibrationFactor );



strType_XPT2046_TouchPara strXPT2046_TouchPara = { 0.085958, -0.001073, -4.979353, -0.001750, 0.065168, -13.318824 };  
                                              // { 0.001030, 0.064188, -10.804098, -0.085584, 0.001420, 324.127036 };  

volatile uint8_t ucXPT2046_TouchFlag = 0;
volatile int function_flag = 0;

volatile uint32_t ADCConvertedValue1;
volatile uint32_t ADCConvertedValue2;


void XPT2046_Init ( void )
{
	XPT2046_GPIO_SPI_Config ();
	
	XPT2046_EXTI_Config ();
		
}


static void XPT2046_EXTI_NVIC_Config ( void )
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


	
static void XPT2046_EXTI_Config ( void )
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


static void XPT2046_GPIO_SPI_Config ( void ) 
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

	
static void XPT2046_DelayUS ( __IO uint32_t ulCount )
{
	uint32_t i;


	for ( i = 0; i < ulCount; i ++ )
	{
		uint8_t uc = 12;     
	      
		while ( uc -- );     

	}
	
}


static void XPT2046_WriteCMD ( uint8_t ucCmd ) 
{
	uint8_t i;


	macXPT2046_MOSI_0();
	
	macXPT2046_CLK_LOW();

	for ( i = 0; i < 8; i ++ ) 
	{
		( ( ucCmd >> ( 7 - i ) ) & 0x01 ) ? macXPT2046_MOSI_1() : macXPT2046_MOSI_0();
		
	  XPT2046_DelayUS ( 5 );
		
		macXPT2046_CLK_HIGH();

	  XPT2046_DelayUS ( 5 );

		macXPT2046_CLK_LOW();
	}
	
}


static uint16_t XPT2046_ReadCMD ( void ) 
{
	uint8_t i;
	uint16_t usBuf=0, usTemp;
	


	macXPT2046_MOSI_0();

	macXPT2046_CLK_HIGH();

	for ( i=0;i<12;i++ ) 
	{
		macXPT2046_CLK_LOW();    
	
		usTemp = macXPT2046_MISO();
		
		usBuf |= usTemp << ( 11 - i );
	
		macXPT2046_CLK_HIGH();
		
	}
	
	return usBuf;

}


static uint16_t XPT2046_ReadAdc ( uint8_t ucChannel )
{
	XPT2046_WriteCMD ( ucChannel );

  return 	XPT2046_ReadCMD ();
	
}


static void XPT2046_ReadAdc_XY ( int16_t * sX_Ad, int16_t * sY_Ad )  
{ 
	int16_t sX_Ad_Temp, sY_Ad_Temp; 

	
	
	sX_Ad_Temp = XPT2046_ReadAdc ( macXPT2046_CHANNEL_X );

	XPT2046_DelayUS ( 1 ); 

	sY_Ad_Temp = XPT2046_ReadAdc ( macXPT2046_CHANNEL_Y ); 
	
	
	* sX_Ad = sX_Ad_Temp; 
	* sY_Ad = sY_Ad_Temp; 
	
	
}


#if   0                 
static uint8_t XPT2046_ReadAdc_Smooth_XY ( strType_XPT2046_Coordinate * pScreenCoordinate )
{
	uint8_t ucCount = 0;
	
	int16_t sAD_X, sAD_Y;
	int16_t sBufferArray [ 2 ] [ 9 ] = { { 0 }, { 0 } };  

	int32_t lAverage  [ 3 ], lDifference [ 3 ];
	

	do
	{		   
		XPT2046_ReadAdc_XY ( & sAD_X, & sAD_Y );
		
		sBufferArray [ 0 ] [ ucCount ] = sAD_X;  
		sBufferArray [ 1 ] [ ucCount ] = sAD_Y;
		
		ucCount ++; 
			 
	} while ( ( macXPT2046_EXTI_Read() == macXPT2046_EXTI_ActiveLevel ) && ( ucCount < 9 ) ); 	//?û?????????????TP_INT_IN?z???? ???? ucCount<9*/
	 
	

	if ( macXPT2046_EXTI_Read() != macXPT2046_EXTI_ActiveLevel )
		ucXPT2046_TouchFlag = 0;			


	if ( ucCount == 9 )   								
	{  

		lAverage  [ 0 ] = ( sBufferArray [ 0 ] [ 0 ] + sBufferArray [ 0 ] [ 1 ] + sBufferArray [ 0 ] [ 2 ] ) / 3;
		lAverage  [ 1 ] = ( sBufferArray [ 0 ] [ 3 ] + sBufferArray [ 0 ] [ 4 ] + sBufferArray [ 0 ] [ 5 ] ) / 3;
		lAverage  [ 2 ] = ( sBufferArray [ 0 ] [ 6 ] + sBufferArray [ 0 ] [ 7 ] + sBufferArray [ 0 ] [ 8 ] ) / 3;
		

		lDifference [ 0 ] = lAverage  [ 0 ]-lAverage  [ 1 ];
		lDifference [ 1 ] = lAverage  [ 1 ]-lAverage  [ 2 ];
		lDifference [ 2 ] = lAverage  [ 2 ]-lAverage  [ 0 ];
		

		lDifference [ 0 ] = lDifference [ 0 ]>0?lDifference [ 0 ]: ( -lDifference [ 0 ] );
		lDifference [ 1 ] = lDifference [ 1 ]>0?lDifference [ 1 ]: ( -lDifference [ 1 ] );
		lDifference [ 2 ] = lDifference [ 2 ]>0?lDifference [ 2 ]: ( -lDifference [ 2 ] );
		

		if (  lDifference [ 0 ] > macXPT2046_THRESHOLD_CalDiff  &&  lDifference [ 1 ] > macXPT2046_THRESHOLD_CalDiff  &&  lDifference [ 2 ] > macXPT2046_THRESHOLD_CalDiff  ) 
			return 0;
		

		if ( lDifference [ 0 ] < lDifference [ 1 ] )
		{
			if ( lDifference [ 2 ] < lDifference [ 0 ] ) 
				pScreenCoordinate ->x = ( lAverage  [ 0 ] + lAverage  [ 2 ] ) / 2;
			else 
				pScreenCoordinate ->x = ( lAverage  [ 0 ] + lAverage  [ 1 ] ) / 2;	
		}
		
		else if ( lDifference [ 2 ] < lDifference [ 1 ] ) 
			pScreenCoordinate -> x = ( lAverage  [ 0 ] + lAverage  [ 2 ] ) / 2;
		
		else 
			pScreenCoordinate ->x = ( lAverage  [ 1 ] + lAverage  [ 2 ] ) / 2;
		
		

		lAverage  [ 0 ] = ( sBufferArray [ 1 ] [ 0 ] + sBufferArray [ 1 ] [ 1 ] + sBufferArray [ 1 ] [ 2 ] ) / 3;
		lAverage  [ 1 ] = ( sBufferArray [ 1 ] [ 3 ] + sBufferArray [ 1 ] [ 4 ] + sBufferArray [ 1 ] [ 5 ] ) / 3;
		lAverage  [ 2 ] = ( sBufferArray [ 1 ] [ 6 ] + sBufferArray [ 1 ] [ 7 ] + sBufferArray [ 1 ] [ 8 ] ) / 3;
		
		lDifference [ 0 ] = lAverage  [ 0 ] - lAverage  [ 1 ];
		lDifference [ 1 ] = lAverage  [ 1 ] - lAverage  [ 2 ];
		lDifference [ 2 ] = lAverage  [ 2 ] - lAverage  [ 0 ];
		

		lDifference [ 0 ] = lDifference [ 0 ] > 0 ? lDifference [ 0 ] : ( - lDifference [ 0 ] );
		lDifference [ 1 ] = lDifference [ 1 ] > 0 ? lDifference [ 1 ] : ( - lDifference [ 1 ] );
		lDifference [ 2 ] = lDifference [ 2 ] > 0 ? lDifference [ 2 ] : ( - lDifference [ 2 ] );
		
		
		if ( lDifference [ 0 ] > macXPT2046_THRESHOLD_CalDiff && lDifference [ 1 ] > macXPT2046_THRESHOLD_CalDiff && lDifference [ 2 ] > macXPT2046_THRESHOLD_CalDiff ) 
			return 0;
		
		if ( lDifference [ 0 ] < lDifference [ 1 ] )
		{
			if ( lDifference [ 2 ] < lDifference [ 0 ] ) 
				pScreenCoordinate ->y =  ( lAverage  [ 0 ] + lAverage  [ 2 ] ) / 2;
			else 
				pScreenCoordinate ->y =  ( lAverage  [ 0 ] + lAverage  [ 1 ] ) / 2;	
		}
		else if ( lDifference [ 2 ] < lDifference [ 1 ] ) 
			pScreenCoordinate ->y =  ( lAverage  [ 0 ] + lAverage  [ 2 ] ) / 2;
		else
			pScreenCoordinate ->y =  ( lAverage  [ 1 ] + lAverage  [ 2 ] ) / 2;
		
		
		return 1;
		
		
	}
	
	else if ( ucCount > 1 )
	{
		pScreenCoordinate ->x = sBufferArray [ 0 ] [ 0 ];
		pScreenCoordinate ->y = sBufferArray [ 1 ] [ 0 ];
	
		return 0;
		
	}  
	
	
	return 0; 
	
	
}


#else     
static uint8_t XPT2046_ReadAdc_Smooth_XY ( strType_XPT2046_Coordinate * pScreenCoordinate )
{
	uint8_t ucCount = 0, i;
	
	int16_t sAD_X, sAD_Y;
	int16_t sBufferArray [ 2 ] [ 10 ] = { { 0 },{ 0 } };  
	
	int32_t lX_Min, lX_Max, lY_Min, lY_Max;


	do					       				
	{		  
		XPT2046_ReadAdc_XY ( & sAD_X, & sAD_Y );  
		
		sBufferArray [ 0 ] [ ucCount ] = sAD_X;  
		sBufferArray [ 1 ] [ ucCount ] = sAD_Y;
		
		ucCount ++;  
		
	}	while ( ( macXPT2046_EXTI_Read() == macXPT2046_EXTI_ActiveLevel ) && ( ucCount < 10 ) );
	

	if ( macXPT2046_EXTI_Read() != macXPT2046_EXTI_ActiveLevel )
		ucXPT2046_TouchFlag = 0;			


	if ( ucCount ==10 )		 					
	{
		lX_Max = lX_Min = sBufferArray [ 0 ] [ 0 ];
		lY_Max = lY_Min = sBufferArray [ 1 ] [ 0 ];       
		
		for ( i = 1; i < 10; i ++ )
		{
			if ( sBufferArray [ 0 ] [ i ] < lX_Min )
				lX_Min = sBufferArray [ 0 ] [ i ];
			
			else if ( sBufferArray [ 0 ] [ i ] > lX_Max )
				lX_Max = sBufferArray [ 0 ] [ i ];

		}
		
		for ( i = 1; i < 10; i ++ )
		{
			if ( sBufferArray [ 1 ] [ i ] < lY_Min )
				lY_Min = sBufferArray [ 1 ] [ i ];
			
			else if ( sBufferArray [ 1 ] [ i ] > lY_Max )
				lY_Max = sBufferArray [ 1 ] [ i ];

		}
		

		pScreenCoordinate ->x =  ( sBufferArray [ 0 ] [ 0 ] + sBufferArray [ 0 ] [ 1 ] + sBufferArray [ 0 ] [ 2 ] + sBufferArray [ 0 ] [ 3 ] + sBufferArray [ 0 ] [ 4 ] + 
		                           sBufferArray [ 0 ] [ 5 ] + sBufferArray [ 0 ] [ 6 ] + sBufferArray [ 0 ] [ 7 ] + sBufferArray [ 0 ] [ 8 ] + sBufferArray [ 0 ] [ 9 ] - lX_Min-lX_Max ) >> 3;
		
		pScreenCoordinate ->y =  ( sBufferArray [ 1 ] [ 0 ] + sBufferArray [ 1 ] [ 1 ] + sBufferArray [ 1 ] [ 2 ] + sBufferArray [ 1 ] [ 3 ] + sBufferArray [ 1 ] [ 4 ] + 
		                           sBufferArray [ 1 ] [ 5 ] + sBufferArray [ 1 ] [ 6 ] + sBufferArray [ 1 ] [ 7 ] + sBufferArray [ 1 ] [ 8 ] + sBufferArray [ 1 ] [ 9 ] - lY_Min-lY_Max ) >> 3; 
		
		
		return 1;
		

	}   
	
	
	return 0;    
	
	
}


#endif



static uint8_t XPT2046_Calculate_CalibrationFactor ( strType_XPT2046_Coordinate * pDisplayCoordinate, strType_XPT2046_Coordinate * pScreenSample, strType_XPT2046_Calibration * pCalibrationFactor )
{
	uint8_t ucRet = 1;


	pCalibrationFactor -> Divider =  ( ( pScreenSample [ 0 ] .x - pScreenSample [ 2 ] .x ) *  ( pScreenSample [ 1 ] .y - pScreenSample [ 2 ] .y ) ) - 
									                 ( ( pScreenSample [ 1 ] .x - pScreenSample [ 2 ] .x ) *  ( pScreenSample [ 0 ] .y - pScreenSample [ 2 ] .y ) ) ;
	
	
	if (  pCalibrationFactor -> Divider == 0  )
		ucRet = 0;

	else
	{

		pCalibrationFactor -> An =  ( ( pDisplayCoordinate [ 0 ] .x - pDisplayCoordinate [ 2 ] .x ) *  ( pScreenSample [ 1 ] .y - pScreenSample [ 2 ] .y ) ) - 
								                ( ( pDisplayCoordinate [ 1 ] .x - pDisplayCoordinate [ 2 ] .x ) *  ( pScreenSample [ 0 ] .y - pScreenSample [ 2 ] .y ) );
		
		pCalibrationFactor -> Bn =  ( ( pScreenSample [ 0 ] .x - pScreenSample [ 2 ] .x ) *  ( pDisplayCoordinate [ 1 ] .x - pDisplayCoordinate [ 2 ] .x ) ) - 
								                ( ( pDisplayCoordinate [ 0 ] .x - pDisplayCoordinate [ 2 ] .x ) *  ( pScreenSample [ 1 ] .x - pScreenSample [ 2 ] .x ) );
		
		pCalibrationFactor -> Cn =  ( pScreenSample [ 2 ] .x * pDisplayCoordinate [ 1 ] .x - pScreenSample [ 1 ] .x * pDisplayCoordinate [ 2 ] .x ) * pScreenSample [ 0 ] .y +
								                ( pScreenSample [ 0 ] .x * pDisplayCoordinate [ 2 ] .x - pScreenSample [ 2 ] .x * pDisplayCoordinate [ 0 ] .x ) * pScreenSample [ 1 ] .y +
								                ( pScreenSample [ 1 ] .x * pDisplayCoordinate [ 0 ] .x - pScreenSample [ 0 ] .x * pDisplayCoordinate [ 1 ] .x ) * pScreenSample [ 2 ] .y ;
		
		pCalibrationFactor -> Dn =  ( ( pDisplayCoordinate [ 0 ] .y - pDisplayCoordinate [ 2 ] .y ) *  ( pScreenSample [ 1 ] .y - pScreenSample [ 2 ] .y ) ) - 
								                ( ( pDisplayCoordinate [ 1 ] .y - pDisplayCoordinate [ 2 ] .y ) *  ( pScreenSample [ 0 ] .y - pScreenSample [ 2 ] .y ) ) ;
		
		pCalibrationFactor -> En =  ( ( pScreenSample [ 0 ] .x - pScreenSample [ 2 ] .x ) *  ( pDisplayCoordinate [ 1 ] .y - pDisplayCoordinate [ 2 ] .y ) ) - 
								                ( ( pDisplayCoordinate [ 0 ] .y - pDisplayCoordinate [ 2 ] .y ) *  ( pScreenSample [ 1 ] .x - pScreenSample [ 2 ] .x ) ) ;
		

		pCalibrationFactor -> Fn =  ( pScreenSample [ 2 ] .x * pDisplayCoordinate [ 1 ] .y - pScreenSample [ 1 ] .x * pDisplayCoordinate [ 2 ] .y ) * pScreenSample [ 0 ] .y +
								                ( pScreenSample [ 0 ] .x * pDisplayCoordinate [ 2 ] .y - pScreenSample [ 2 ] .x * pDisplayCoordinate [ 0 ] .y ) * pScreenSample [ 1 ] .y +
								                ( pScreenSample [ 1 ] .x * pDisplayCoordinate [ 0 ] .y - pScreenSample [ 0 ] .x * pDisplayCoordinate [ 1 ] .y ) * pScreenSample [ 2 ] .y;
			
	}
	
	
	return ucRet;
	
	
}


//Calibrate touchscreen and be a menu of the project
uint8_t XPT2046_Touch_Calibrate ( void )
{
	#if 1
		uint8_t i;
		
		char cStr [ 10 ];
		
    uint16_t usScreenWidth, usScreenHeigth;
		uint16_t usTest_x = 0, usTest_y = 0, usGap_x = 0, usGap_y = 0;
		
	  char * pStr = 0;
	
    strType_XPT2046_Coordinate strCrossCoordinate [ 4 ], strScreenSample [ 4 ];
	  
	  strType_XPT2046_Calibration CalibrationFactor;
    		

		#if ( macXPT2046_Coordinate_GramScan == 1 ) || ( macXPT2046_Coordinate_GramScan == 4 )
	    usScreenWidth = LCD_Default_Max_Width;
	    usScreenHeigth = LCD_Default_Max_Heigth;

	  #elif ( macXPT2046_Coordinate_GramScan == 2 ) || ( macXPT2046_Coordinate_GramScan == 3 )
	    usScreenWidth = LCD_Default_Max_Heigth;
	    usScreenHeigth = LCD_Default_Max_Width;
	
	  #endif
		
		

		strCrossCoordinate [ 0 ] .x = usScreenWidth >> 2;
		strCrossCoordinate [ 0 ] .y = usScreenHeigth >> 2;
		
		strCrossCoordinate [ 1 ] .x = strCrossCoordinate [ 0 ] .x;
		strCrossCoordinate [ 1 ] .y = ( usScreenHeigth * 3 ) >> 2;
		
		strCrossCoordinate [ 2 ] .x = ( usScreenWidth * 3 ) >> 2;
		strCrossCoordinate [ 2 ] .y = strCrossCoordinate [ 1 ] .y;
		
		strCrossCoordinate [ 3 ] .x = strCrossCoordinate [ 2 ] .x;
		strCrossCoordinate [ 3 ] .y = strCrossCoordinate [ 0 ] .y;		
	  	
			
		LCD_GramScan (1 );
		
		
		for ( i = 0; i < 4; i ++ )
		{ 
			LCD_Clear ( 0, 0, usScreenWidth, usScreenHeigth, YELLOW );       
			
			//pStr = "Welcome to the Smart Street Lamp System!";		
			//LCD_DrawString_Color(80, 100, pStr, YELLOW, BLACK);
			pStr = "Calibrate Screen...";
      LCD_DrawString_Color ( ( usScreenWidth - ( strlen ( pStr ) - 7 ) * WIDTH_EN_CHAR ) >> 1, usScreenHeigth >> 1, pStr, YELLOW, BLACK );			
		
			sprintf ( cStr, "%d", i + 1 );
			//char* pStr1 = 0;
			//pStr1 = "NEXT";
			//LCD_DrawString_Color(80, 200, pStr1, YELLOW, BLACK);
			LCD_DrawString_Color ( usScreenWidth >> 1, ( usScreenHeigth >> 1 ) - HEIGHT_EN_CHAR, cStr, YELLOW, BLACK );
		
			XPT2046_DelayUS ( 100000 );		                                                   
			
			LCD_DrawCross ( strCrossCoordinate [ i ] .x, strCrossCoordinate [ i ] .y );  

			while ( ! XPT2046_ReadAdc_Smooth_XY ( & strScreenSample [ i ] ) );         

		}
		
		
		XPT2046_Calculate_CalibrationFactor ( strCrossCoordinate, strScreenSample, & CalibrationFactor ) ;  	 
		
		if ( CalibrationFactor .Divider == 0 ) goto Failure;
		
			
		usTest_x = ( ( CalibrationFactor .An * strScreenSample [ 3 ] .x ) + ( CalibrationFactor .Bn * strScreenSample [ 3 ] .y ) + CalibrationFactor .Cn ) / CalibrationFactor .Divider;		
		usTest_y = ( ( CalibrationFactor .Dn * strScreenSample [ 3 ] .x ) + ( CalibrationFactor .En * strScreenSample [ 3 ] .y ) + CalibrationFactor .Fn ) / CalibrationFactor .Divider;   
		
		usGap_x = ( usTest_x > strCrossCoordinate [ 3 ] .x ) ? ( usTest_x - strCrossCoordinate [ 3 ] .x ) : ( strCrossCoordinate [ 3 ] .x - usTest_x );  
		usGap_y = ( usTest_y > strCrossCoordinate [ 3 ] .y ) ? ( usTest_y - strCrossCoordinate [ 3 ] .y ) : ( strCrossCoordinate [ 3 ] .y - usTest_y );  
		
    if ( ( usGap_x > 10 ) || ( usGap_y > 10 ) ) goto Failure;       
		

		strXPT2046_TouchPara .dX_X = ( CalibrationFactor .An * 1.0 ) / CalibrationFactor .Divider;
		strXPT2046_TouchPara .dX_Y = ( CalibrationFactor .Bn * 1.0 ) / CalibrationFactor .Divider;
		strXPT2046_TouchPara .dX   = ( CalibrationFactor .Cn * 1.0 ) / CalibrationFactor .Divider;
		
		strXPT2046_TouchPara .dY_X = ( CalibrationFactor .Dn * 1.0 ) / CalibrationFactor .Divider;
		strXPT2046_TouchPara .dY_Y = ( CalibrationFactor .En * 1.0 ) / CalibrationFactor .Divider;
		strXPT2046_TouchPara .dY   = ( CalibrationFactor .Fn * 1.0 ) / CalibrationFactor .Divider;

	

	#endif
	
	
	LCD_Clear ( 0, 0, usScreenWidth, usScreenHeigth, YELLOW );
	
	pStr = "Calibrate Succed";			
	LCD_DrawString_Color ( ( usScreenWidth - strlen ( pStr ) * WIDTH_EN_CHAR ) >> 1, usScreenHeigth >> 1, pStr, BACKGROUND, RED );	

  XPT2046_DelayUS ( 200000 );

	return 1;    
	

	Failure:
	
	LCD_Clear ( 0, 0, usScreenWidth, usScreenHeigth, BACKGROUND ); 
	
	pStr = "Calibrate fail";			
	LCD_DrawString_Color ( ( usScreenWidth - strlen ( pStr ) * WIDTH_EN_CHAR ) >> 1, usScreenHeigth >> 1, pStr, BACKGROUND, RED );	

	pStr = "try again";			
	LCD_DrawString_Color ( ( usScreenWidth - strlen ( pStr ) * WIDTH_EN_CHAR ) >> 1, ( usScreenHeigth >> 1 ) + HEIGHT_EN_CHAR, pStr, BACKGROUND, RED );				

	XPT2046_DelayUS ( 1000000 );		
	
	return 0; 
		
		
}

   

uint8_t XPT2046_Get_TouchedPoint ( strType_XPT2046_Coordinate * pDisplayCoordinate, strType_XPT2046_TouchPara * pTouchPara )
{
	uint8_t ucRet = 1;           
	
	strType_XPT2046_Coordinate strScreenCoordinate; 
	

  if ( XPT2046_ReadAdc_Smooth_XY ( & strScreenCoordinate ) )
  {    
		pDisplayCoordinate ->x = ( ( pTouchPara ->dX_X * strScreenCoordinate .x ) + ( pTouchPara ->dX_Y * strScreenCoordinate .y ) + pTouchPara ->dX );        
		pDisplayCoordinate ->y = ( ( pTouchPara ->dY_X * strScreenCoordinate .x ) + ( pTouchPara ->dY_Y * strScreenCoordinate .y ) + pTouchPara ->dY );

  }
	 
	else ucRet = 0;            
	
	return ucRet;
	
	
} 

void display_co(void){
	strType_XPT2046_Coordinate strDisplayCoordinate;
	char s1[10];
	char s2[10];
		if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) ){
			sprintf(s1, "%d",  strDisplayCoordinate .x);
			sprintf(s2, "%d", strDisplayCoordinate .y);
			LCD_DrawString(20, 220, "X coordinate: ");
			LCD_DrawString(130, 220, s1);
			LCD_DrawString(20, 240, "Y coordinate: ");
			LCD_DrawString(130, 240, s2);
		}
	
}


void Next_touchkey(void)
{
		strType_XPT2046_Coordinate strDisplayCoordinate;
	
	if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 100 ) && ( strDisplayCoordinate .y < 140 ) )
		{
			if ( ( strDisplayCoordinate .x > 35 ) && ( strDisplayCoordinate .x < 50 ) )
			{
					function_flag = 1;
			}					
		}

	}

}

void Menu_conf(void){
				LCD_GramScan ( 1 );
				LCD_Clear(0, 0, 240, 320, BACKGROUND );
				LCD_DrawString(20, 30, "SELECT YOUR DESIRED MENU");
				LCD_DrawString(20, 60, "1. Digital Clock");
				LCD_DrawString(20, 80, "2. Auto-Control");
				LCD_DrawString(20, 100, "3. Manual Control");
				LCD_DrawString(20, 120, "4. Temperature & Humidity");
				LCD_Clear(30,  170,  50, 50, GREEN); //Button 1
				LCD_Clear(160, 170, 50, 50, GREEN); //Button 2
				LCD_Clear(30, 250, 50, 50, GREEN); // Button 3
				LCD_Clear(160, 250, 50, 50, GREEN); //Button 4
}

void Menu_touch(void){
	
	strType_XPT2046_Coordinate strDisplayCoordinate;
	
	//1. Digital Clock
	if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 40 ) && ( strDisplayCoordinate .y < 55 ) )
		{
			if ( ( strDisplayCoordinate .x > 120 ) && ( strDisplayCoordinate .x < 140 ) )
			{
				function_flag = 2;
			}					
		}
	}
	
	//2. Auto-Control
		if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 175 ) && ( strDisplayCoordinate .y < 190 ) )
		{
			if ( ( strDisplayCoordinate .x > 115 ) && ( strDisplayCoordinate .x < 140 ) )
			{
				function_flag = 3;
			}					
		}
	}
	
		//3. Manual Control
		if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 35 ) && ( strDisplayCoordinate .y < 65 ) )
		{
			if ( ( strDisplayCoordinate .x > 45 ) && ( strDisplayCoordinate .x < 65 ) )
			{
				function_flag = 4;
			}					
		}
	}
	
	//4. Temperature & Humidity
			if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 165 ) && ( strDisplayCoordinate .y < 190 ) )
		{
			if ( ( strDisplayCoordinate .x > 45 ) && ( strDisplayCoordinate .x < 60 ) )
			{
				function_flag = 5;
			}					
		}
	}
		
}

int Digital_clock(){
		
		strType_XPT2046_Coordinate strDisplayCoordinate;
	
	while(1){
	
			Time_Get();
	
		//Time adjustment
				if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 70 ) && ( strDisplayCoordinate .y < 150 ) )
		{
			if ( ( strDisplayCoordinate .x > 125) && ( strDisplayCoordinate .x <135 ) )
			{
				return function_flag = 10;
			}					
		}
	}
	/*
			//Date adjustment
	if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 65 ) && ( strDisplayCoordinate .y < 150 ) )
		{
			if ( ( strDisplayCoordinate .x > 70 ) && ( strDisplayCoordinate .x < 95 ) )
			{
				return function_flag = 11;
			}					
		}
	}*/
	
			//When pressing 'Back' button
	if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 100 ) && ( strDisplayCoordinate .y < 140 ) )
		{
			if ( ( strDisplayCoordinate .x > 35 ) && ( strDisplayCoordinate .x < 50 ) )
			{
				return function_flag = 1;
			}					
		}
	}
}
	
}


void Clock_config(){
			LCD_GramScan ( 1 );
			LCD_Clear ( 0, 0, 240, 320, BACKGROUND );
			LCD_DrawString(75, 80, "Digital Clock");
			LCD_DrawString(15, 100, "Check and Adjust the clock!");
			LCD_DrawString(15, 125, "Time is now: ");
			LCD_DrawString(15, 140, "Today's date is: ");
			LCD_DrawString(95, 280, "BACK");
			LCD_DrawString(70, 180, "Adjust Time");
			//LCD_DrawString(70, 230, "Adjust Date");
			Digital_clock();

}


int led_intentsity_range(){
	
	strType_XPT2046_Coordinate strDisplayCoordinate;
	
	while(1){
		
		//LDR_value();
		
		if(ADCConvertedValue1 <= 3890){
			TIM3->CCR1 = 0;
		}
		else if(ADCConvertedValue1 > 3900 && ADCConvertedValue1 <= 3919){
			TIM3->CCR1 = 7035;
		}	
		else if(ADCConvertedValue1 > 3920 && ADCConvertedValue1 <= 3939){
			TIM3->CCR1 = 13535;
		}	
		else if(ADCConvertedValue1 > 3940 && ADCConvertedValue1 <= 3949){
			TIM3->CCR1 = 20035;
		}	
		else if(ADCConvertedValue1 > 3950 && ADCConvertedValue1 <= 3969){
			TIM3->CCR1 = 26535;
		}
		else if(ADCConvertedValue1 > 3970 && ADCConvertedValue1 <= 3989){
			TIM3->CCR1 = 33035;
		}	
		else if(ADCConvertedValue1 > 3990 && ADCConvertedValue1 <= 4009){
			TIM3->CCR1 = 39535;
		}	
		else if(ADCConvertedValue1 > 4010 && ADCConvertedValue1 <= 4029){
			TIM3->CCR1 = 46035;
		}	
		else if(ADCConvertedValue1 > 4030 && ADCConvertedValue1 <= 4049){
			TIM3->CCR1 = 52535;
		}	
		else if(ADCConvertedValue1 > 4050 && ADCConvertedValue1 <= 4069){
			TIM3->CCR1 = 59035;
		}	
		else if(ADCConvertedValue1 > 4070 && ADCConvertedValue1 <= 4095){
			TIM3->CCR1 = 65535;
		}		
		
					//When pressing 'Back' button
	if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 100 ) && ( strDisplayCoordinate .y < 140 ) )
		{
			if ( ( strDisplayCoordinate .x > 35 ) && ( strDisplayCoordinate .x < 50 ) )
			{
				return function_flag = 1;
			}					
		}
	}
	
	}
}

int passive_control(){
	
		char test[10];
		strType_XPT2046_Coordinate strDisplayCoordinate;
		TIM3->CCR1 = 0;
	
	
		while(1){
			
			//Pressing "+" button
		if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 40 ) && ( strDisplayCoordinate .y < 60 ) )
		{
			if ( ( strDisplayCoordinate .x > 110 ) && ( strDisplayCoordinate .x < 135 ) )
			{
				TIM3->CCR1 += 3000;
				sprintf(test, "%d", TIM3->CCR1);
				//LCD_DrawString(80, 110, test); //Debugging
				delay_m(300);
		}
			
			}					
		}
	
					//Pressing "-" button
		if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 165 ) && ( strDisplayCoordinate .y < 190 ) )
		{
			if ( ( strDisplayCoordinate .x > 105) && ( strDisplayCoordinate .x < 130 ) )
			{
				TIM3->CCR1 -= 3000;
				
				sprintf(test, "%d", TIM3->CCR1);
				//LCD_DrawString(80, 110, test); //Debugging
				delay_m(300);
		}
			
			}					
		}
	
				//When pressing 'Back' button
	if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 100 ) && ( strDisplayCoordinate .y < 140 ) )
		{
			if ( ( strDisplayCoordinate .x > 35 ) && ( strDisplayCoordinate .x < 50 ) )
			{
				return function_flag = 1;
			}					
		}
	}
	
	}
			
}

uint8_t custom_temp = 0;
uint8_t custom_hum = 0;
char stringTemp[10];
char stringHum[10];

int dht11_dataRead(){
	
	strType_XPT2046_Coordinate strDisplayCoordinate;
	DHT11_Start();
	while(1){
		
		if(DHT11_Check()){
		sprintf(stringTemp, "%d", DHT11_GetTemperature());
		sprintf(stringHum, "%d", DHT11_GetHumidity());
		LCD_DrawString(175, 100, stringTemp);
		LCD_DrawString(145, 120, stringHum);
		}
	 else {
		LCD_DrawString(60, 140, "Data request failed");
	}
	delay_m(1000);

	//When pressing 'Back' button
	if ( XPT2046_Get_TouchedPoint ( & strDisplayCoordinate, & strXPT2046_TouchPara ) )
	{
		if ( ( strDisplayCoordinate .y > 90) && ( strDisplayCoordinate .y < 160 ))
		{
			if ( ( strDisplayCoordinate .x > 35 ) && ( strDisplayCoordinate .x < 60) )
			{
				return function_flag = 1;
			}					
		}
	}
}
	
}

void auto_light_mode(){
		LCD_GramScan ( 1 );
		LCD_Clear ( 0, 0, 240, 320, BACKGROUND );
		LCD_DrawString(20, 100, "Auto Control is working!");
		LCD_DrawString(95, 280, "BACK");
		led_intentsity_range();
}


void manual_control_mode(){
		LCD_GramScan ( 1 );
		LCD_Clear ( 0, 0, 240, 320, BACKGROUND );
		LCD_DrawString(20, 60, "Manual Control is working!");
		LCD_DrawString(10, 90, "+ and - to control the light");
		LCD_DrawString(30, 150, "Brighter");
		LCD_DrawString(160, 150, "Darker");
		LCD_Clear(30, 180, 50, 50, CYAN);
		LCD_Clear(160, 180, 50, 50, CYAN);
		LCD_DrawString(95, 280, "BACK");
		passive_control();
		
}

void dht11_display(){
		LCD_GramScan ( 1 );
		LCD_Clear ( 0, 0, 240, 320, BACKGROUND );
		LCD_DrawString(35, 60, "Temperature & Humidity");
		LCD_DrawString(95, 280, "BACK");
		dht11_dataRead();
}


