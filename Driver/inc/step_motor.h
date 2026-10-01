#ifndef _step_motor_H
#define _step_motor_H
#include "system.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "misc.h" 
// 定义步进电机速度，值越小，速度越快
// 最小不能小于1
#define STEPMOTOR_MAXSPEED        1  
#define STEPMOTOR_MINSPEED        5  

//========= 第一路电机【原有代码，完全不变 PC0~PC3】=========
#define MOTOR_IN1_PORT 			GPIOC  
#define MOTOR_IN1_PIN 			GPIO_Pin_0
#define MOTOR_IN1_PORT_RCC		RCC_AHB1Periph_GPIOC
#define MOTOR_IN2_PORT 			GPIOC  
#define MOTOR_IN2_PIN 			GPIO_Pin_1
#define MOTOR_IN2_PORT_RCC		RCC_AHB1Periph_GPIOC
#define MOTOR_IN3_PORT 			GPIOC  
#define MOTOR_IN3_PIN 			GPIO_Pin_2
#define MOTOR_IN3_PORT_RCC		RCC_AHB1Periph_GPIOC
#define MOTOR_IN4_PORT 			GPIOC  
#define MOTOR_IN4_PIN 			GPIO_Pin_3
#define MOTOR_IN4_PORT_RCC		RCC_AHB1Periph_GPIOC
//位带定义
#define MOTOR_IN1 	PCout(0)
#define MOTOR_IN2 	PCout(1)
#define MOTOR_IN3 	PCout(2)
#define MOTOR_IN4 	PCout(3)

//========= 新增第二路电机 PC4~PC7【两路独立】=========
#define MOTOR2_IN1_PORT 		GPIOC  
#define MOTOR2_IN1_PIN 			GPIO_Pin_4
#define MOTOR2_IN1_PORT_RCC	RCC_AHB1Periph_GPIOC
#define MOTOR2_IN2_PORT 		GPIOC  
#define MOTOR2_IN2_PIN 			GPIO_Pin_5
#define MOTOR2_IN2_PORT_RCC	RCC_AHB1Periph_GPIOC
#define MOTOR2_IN3_PORT 		GPIOC  
#define MOTOR2_IN3_PIN 			GPIO_Pin_6
#define MOTOR2_IN3_PORT_RCC	RCC_AHB1Periph_GPIOC
#define MOTOR2_IN4_PORT 		GPIOC  
#define MOTOR2_IN4_PIN 			GPIO_Pin_7
#define MOTOR2_IN4_PORT_RCC	RCC_AHB1Periph_GPIOC

#define MOTOR2_IN1 	PCout(4)
#define MOTOR2_IN2 	PCout(5)
#define MOTOR2_IN3 	PCout(6)
#define MOTOR2_IN4 	PCout(7)

//==================== 函数声明【原有不动 + 新增验收用中断接口】====================
void STEP_Motor_Init(void);
void Step_Motor_Run(u8 step,u8 dir,u8 speed,u16 angle,u8 sta);

//==== 新增 验收需要的 API（TIM6中断驱动，非阻塞，支持两路独立+软件急停）====
void STEP_Motor2_Init(void);
void Motor_Set(uint8_t motor_id,uint8_t dir,uint16_t step_cnt);
void Motor_EmergencyStop(void);
extern uint8_t motor_emergency_stop;
#endif
