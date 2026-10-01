/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"


#include "system.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );

/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
	//STM32串口 接收中断使能
	if(xRxEnable==TRUE)   
	{   
		//使能接收和接收中断  
		USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);  
		GPIO_ResetBits(GPIOG,GPIO_Pin_8);//RS485驱动芯片，接收使能
	} 
	else 
	{
		//禁止接收和接收中断    
		USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
		GPIO_SetBits(GPIOG,GPIO_Pin_8);//RS485驱动芯片，发送使能
	}
	//STM32串口 发送中断使能  
	if(xTxEnable==TRUE)   
	{  
		//使能发送完成中断  
		USART_ITConfig(USART2, USART_IT_TXE, ENABLE);  
	}   
	else  
	{  
		//禁止发送完成中断  
		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);  
	}  
}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	(void)ucPORT;     //用串口2,不修改串口
	(void)ucDataBits; //不修改数据位长度
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOG,ENABLE); //使能GPIOA\G时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2时钟
	
	//串口2引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); //GPIOA2复用为USART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //GPIOA3复用为USART2
	
	//USART2    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //GPIOA2与GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA2，PA3
	
	//PG8推挽输出，485模式控制  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //GPIOG8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOG,&GPIO_InitStructure); //初始化PG8
	
	//USART2 初始化设置
	USART_InitStructure.USART_BaudRate = ulBaudRate;//波特率设置
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	switch(eParity)
	{
		case MB_PAR_NONE:
		{
			USART_InitStructure.USART_Parity =USART_Parity_No;
			USART_InitStructure.USART_WordLength = USART_WordLength_8b;
			USART_ITConfig(USART2, USART_IT_PE, DISABLE);//关闭奇偶校验错中断
			break;
		}
		case MB_PAR_ODD: 
		{
			USART_InitStructure.USART_Parity =USART_Parity_Odd;
			USART_InitStructure.USART_WordLength = USART_WordLength_9b;
			USART_ITConfig(USART2, USART_IT_PE, ENABLE);//使能奇偶校验错中断
			break;
		}
		case MB_PAR_EVEN:
		{
			USART_InitStructure.USART_Parity =USART_Parity_Even;
			USART_InitStructure.USART_WordLength = USART_WordLength_9b;
			USART_ITConfig(USART2, USART_IT_PE, ENABLE);//使能奇偶校验错中断
			break;
		}
		default:break;
	}
	USART_Init(USART2, &USART_InitStructure); //初始化串口2
	USART_Cmd(USART2, ENABLE);  //使能串口 2
	
	//Usart2 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
	
	return TRUE;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
	USART_SendData(USART2, ucByte);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
	*pucByte = USART_ReceiveData(USART2);
    return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR( void )
{
    pxMBFrameCBByteReceived(  );
}

void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2, USART_IT_PE) == SET)
	{
		eMBEnable(); //初始化接收
		USART_ClearITPendingBit(USART2, USART_IT_PE);		
	}
	else
	{
		if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
		{
			prvvUARTRxISR();//接受中断
			USART_ClearITPendingBit(USART2, USART_IT_RXNE);		
		}
		if(USART_GetITStatus(USART2, USART_IT_TXE) == SET)
		{
			prvvUARTTxReadyISR();//发送完成中断
			USART_ClearITPendingBit(USART2, USART_IT_TXE);
		}
	}
	//溢出-如果发生溢出需要先读SR,再读DR寄存器 则可清除不断进入中断的问题
	if(USART_GetFlagStatus(USART2,USART_FLAG_ORE)==SET)
	{
		USART_ClearFlag(USART2,USART_FLAG_ORE);	//读SR
		USART_ReceiveData(USART2);				//读DR
	}		
}
