#ifndef __ADC_H
#define __ADC_H
#include "stm32f4xx_adc.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_dma.h"

#define ADC_CH_NUM 4
extern uint16_t adc_buf[ADC_CH_NUM];
extern uint16_t adc_filter[ADC_CH_NUM];

void ADC1_DMA_Init(void);
uint16_t ADC_Filter(uint16_t new_val, uint16_t old_val);

#endif
