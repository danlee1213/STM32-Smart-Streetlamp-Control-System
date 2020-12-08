# STM32-Smart-Streetlamp-Control-System
Using STM32103FVE to build the Smart Streetlamp Control System

In order to emphasize its "ecofriendly", STM32 board will only be operated by the external solar power energy circuit.

__Using STM32 Standard Peripheral Libraries__

# Overview
This project aims to build the Smart Streetlamp Control System using solar power system.

STM32 Cortex-M3 provides following functions:
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



```C
void XPT2046_Init(void){
  XPT2046_GPIO_SPI_Config();
  XPT2046_EXTI_Config();
}
```


## Ditigal Clock
STM32 provides built-in RTC (Real Time Clock) function.

## Auto-light control
Using LDR and ADC to digitize analog signal from LDR



