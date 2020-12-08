#ifndef __ADC_H

void init_adc1(void);
void init_adc2(void);
void LDR_value(void);

#define ADC1_DR_Address    ((uint32_t)0x4001244C)
#define ADC3_DR_Address    ((uint32_t)0x40013C4C)
#endif
