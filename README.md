# STM32-Smart-Streetlamp-Control-System
Using STM32103FVE to build the Smart Streetlamp Control System

STM32 board will only be operated by the external solar power energy circuit.

__Using STM32 Standard Peripheral Libraries__

# Overview
This project aims to build the Smart Streetlamp Control System
It has following functions:
1. Touchscreen function using TFT LCD
2. Real Time Digital Clock
3. Auto-light control mode
4. Manual ligt control mode
5. Temperature & Humidity measurement

## Solar Power Energy Circuit Schematic
<div>
  <img src = https://user-images.githubusercontent.com/72503871/101485489-31e68280-3996-11eb-8bcf-51b2a8b7ade8.png width="450">
  <img src = https://user-images.githubusercontent.com/72503871/101485306-efbd4100-3995-11eb-82b1-7d0dcb99adf8.png>
</div>

## TFT LCD Touchscreen
Using 3.2” TFT LCD Display. Built-in XPT2046 chip triggers the touchscreen function.

<img src = "https://user-images.githubusercontent.com/72503871/101482298-7ae80800-3991-11eb-869c-d2be5261883a.png">

```C
void XPT2046_Init(void){
  XPT2046_GPIO_SPI_Config();
  XPT2046_EXTI_Config();
}
```

<img src = "https://user-images.githubusercontent.com/72503871/101474051-8cc3ae00-3985-11eb-8ae1-3368b6ee2725.jpg">

## Ditigal Clock
STM32 provides built-in RTC (Real Time Clock) function.

## Auto-light control
Using LDR and ADC to digitize analog signal from LDR



