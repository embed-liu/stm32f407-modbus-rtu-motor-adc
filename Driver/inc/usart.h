#ifndef __usart_H
#define __usart_H

#include "system.h" 
#include "stdio.h"
#include "misc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"
#define USART1_REC_LEN		200  	//定义最大接收字节数 200

extern uint8_t  USART1_RX_BUF[USART1_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern uint16_t USART1_RX_STA;         		//接收状态标记

void USART1_Init(u32 bound);


#endif


