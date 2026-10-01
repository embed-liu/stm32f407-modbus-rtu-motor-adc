#include "step_motor.h"
#include "SysTick.h"
#include "stm32f4xx_tim.h"

//========= 全局变量：两路电机独立状态、急停标志（新增）=========
uint8_t motor_emergency_stop = 0;

//电机1
uint16_t motor1_remain_step = 0;
uint8_t motor1_dir = 0;
uint8_t motor1_phase_idx = 0;
//电机2
uint16_t motor2_remain_step = 0;
uint8_t motor2_dir = 0;
uint8_t motor2_phase_idx = 0;

//8拍相序表，沿用你原有相序
uint8_t motor_phase_table[8][4] = {
	{1,0,0,0},
	{1,1,0,0},
	{0,1,0,0},
	{0,1,1,0},
	{0,0,1,0},
	{0,0,1,1},
	{0,0,0,1},
	{1,0,0,1}
};

/*******************************************************************************
* 函 数 名       : STEP_Motor_Init
* 函数功能		 : 一路步进电机初始化
*******************************************************************************/
void STEP_Motor_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//定义结构体变量
	
	RCC_AHB1PeriphClockCmd(MOTOR_IN1_PORT_RCC,ENABLE);
	RCC_AHB1PeriphClockCmd(MOTOR_IN2_PORT_RCC,ENABLE);
	RCC_AHB1PeriphClockCmd(MOTOR_IN3_PORT_RCC,ENABLE);
	RCC_AHB1PeriphClockCmd(MOTOR_IN4_PORT_RCC,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=MOTOR_IN1_PIN;  //选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;	 //设置推挽输出模式
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;	  //设置传输速率
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;//上拉
	GPIO_Init(MOTOR_IN1_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=MOTOR_IN2_PIN;
	GPIO_Init(MOTOR_IN2_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=MOTOR_IN3_PIN;
	GPIO_Init(MOTOR_IN3_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=MOTOR_IN4_PIN;
	GPIO_Init(MOTOR_IN4_PORT,&GPIO_InitStructure);
}

/*******************************************************************************
* 函 数 名       : Step_Motor_Run
* 函数功能		 : 原版阻塞式电机运行
*******************************************************************************/
void Step_Motor_Run(uint8_t step,uint8_t dir,uint8_t speed,uint16_t angle,uint8_t sta)
{
	char i=0;
	uint16_t j=0;
	if(sta==1)
	{
		if(dir==0)	//如果为逆时针旋转
		{
			for(j=0;j<64*angle/45;j++) 
			{
				for(i=0;i<8;i+=(8/step))
				{
					switch(i)//8个节拍控制：A->AB->B->BC->C->CD->D->DA
					{
						case 0: MOTOR_IN1=1;MOTOR_IN2=0;MOTOR_IN3=0;MOTOR_IN4=0;break;
						case 1: MOTOR_IN1=1;MOTOR_IN2=1;MOTOR_IN3=0;MOTOR_IN4=0;break;
						case 2: MOTOR_IN1=0;MOTOR_IN2=1;MOTOR_IN3=0;MOTOR_IN4=0;break;
						case 3: MOTOR_IN1=0;MOTOR_IN2=1;MOTOR_IN3=1;MOTOR_IN4=0;break;
						case 4: MOTOR_IN1=0;MOTOR_IN2=0;MOTOR_IN3=1;MOTOR_IN4=0;break;
						case 5: MOTOR_IN1=0;MOTOR_IN2=0;MOTOR_IN3=1;MOTOR_IN4=1;break;
						case 6: MOTOR_IN1=0;MOTOR_IN2=0;MOTOR_IN3=0;MOTOR_IN4=1;break;
						case 7: MOTOR_IN1=1;MOTOR_IN2=0;MOTOR_IN3=0;MOTOR_IN4=1;break;	
					}
					delay_ms(speed);		
				}	
			}
		}
		else	//如果为顺时针旋转
		{
			for(j=0;j<64*angle/45;j++)
			{
				for(i=0;i<8;i+=(8/step))
				{
					switch(i)//8个节拍控制：A->AB->B->BC->C->CD->D->DA
					{
						case 0: MOTOR_IN1=1;MOTOR_IN2=0;MOTOR_IN3=0;MOTOR_IN4=1;break;
						case 1: MOTOR_IN1=0;MOTOR_IN2=0;MOTOR_IN3=0;MOTOR_IN4=1;break;
						case 2: MOTOR_IN1=0;MOTOR_IN2=0;MOTOR_IN3=1;MOTOR_IN4=1;break;
						case 3: MOTOR_IN1=0;MOTOR_IN2=0;MOTOR_IN3=1;MOTOR_IN4=0;break;
						case 4: MOTOR_IN1=0;MOTOR_IN2=1;MOTOR_IN3=1;MOTOR_IN4=0;break;
						case 5: MOTOR_IN1=0;MOTOR_IN2=1;MOTOR_IN3=0;MOTOR_IN4=0;break;
						case 6: MOTOR_IN1=1;MOTOR_IN2=1;MOTOR_IN3=0;MOTOR_IN4=0;break;
						case 7: MOTOR_IN1=1;MOTOR_IN2=0;MOTOR_IN3=0;MOTOR_IN4=0;break;	
					}
					delay_ms(speed);		
				}	
			}	
		}		
	}
	else
	{
		MOTOR_IN1=0;MOTOR_IN2=0;MOTOR_IN3=0;MOTOR_IN4=0;	
	}		
}

//===================== 第二路电机初始化 + TIM6 + 验收API =====================
void STEP_Motor2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(MOTOR2_IN1_PORT_RCC,ENABLE);
	RCC_AHB1PeriphClockCmd(MOTOR2_IN2_PORT_RCC,ENABLE);
	RCC_AHB1PeriphClockCmd(MOTOR2_IN3_PORT_RCC,ENABLE);
	RCC_AHB1PeriphClockCmd(MOTOR2_IN4_PORT_RCC,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=MOTOR2_IN1_PIN;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_Init(MOTOR2_IN1_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=MOTOR2_IN2_PIN;
	GPIO_Init(MOTOR2_IN2_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=MOTOR2_IN3_PIN;
	GPIO_Init(MOTOR2_IN3_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=MOTOR2_IN4_PIN;
	GPIO_Init(MOTOR2_IN4_PORT,&GPIO_InitStructure);
}

//TIM6初始化，用于产生节拍中断
void TIM6_Init(uint16_t prescaler,uint16_t period)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);
	
	TIM_TimeBaseStructure.TIM_Prescaler = prescaler;
	TIM_TimeBaseStructure.TIM_Period = period;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM6,&TIM_TimeBaseStructure);
	
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM6,ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//电机控制入口：motor_id 1/2；dir:0逆时针，1顺时针；step_cnt：需要走多少拍
void Motor_Set(uint8_t motor_id,uint8_t dir,uint16_t step_cnt)
{
	motor_emergency_stop = 0;
	if(motor_id == 1)
	{
		motor1_remain_step = step_cnt;
		motor1_dir = dir;
		motor1_phase_idx = 0;
	}
	else if(motor_id ==2)
	{
		motor2_remain_step = step_cnt;
		motor2_dir = dir;
		motor2_phase_idx = 0;
	}
}
//全局软件急停
void Motor_EmergencyStop(void)
{
	motor_emergency_stop = 1;
	motor1_remain_step = 0;
	motor2_remain_step = 0;
	MOTOR_IN1=0;MOTOR_IN2=0;MOTOR_IN3=0;MOTOR_IN4=0;
	MOTOR2_IN1=0;MOTOR2_IN2=0;MOTOR2_IN3=0;MOTOR2_IN4=0;
}



void TIM6_DAC_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM6,TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM6,TIM_IT_Update);
		if(motor_emergency_stop == 1)
		{
			return;
		}
		//电机1输出
		if(motor1_remain_step>0)
		{
			if(motor1_dir ==0)
			{
				MOTOR_IN1 = motor_phase_table[motor1_phase_idx][0];
				MOTOR_IN2 = motor_phase_table[motor1_phase_idx][1];
				MOTOR_IN3 = motor_phase_table[motor1_phase_idx][2];
				MOTOR_IN4 = motor_phase_table[motor1_phase_idx][3];
				motor1_phase_idx++;
				if(motor1_phase_idx >=8) motor1_phase_idx=0;
			}
			else
			{
				MOTOR_IN1 = motor_phase_table[motor1_phase_idx][0];
				MOTOR_IN2 = motor_phase_table[motor1_phase_idx][1];
				MOTOR_IN3 = motor_phase_table[motor1_phase_idx][2];
				MOTOR_IN4 = motor_phase_table[motor1_phase_idx][3];
				motor1_phase_idx--;
				if(motor1_phase_idx ==0) motor1_phase_idx=7;
			}
			motor1_remain_step--;
		}
		else
		{
			MOTOR_IN1=0;MOTOR_IN2=0;MOTOR_IN3=0;MOTOR_IN4=0;
		}
		//电机2输出
		if(motor2_remain_step>0)
		{
			if(motor2_dir ==0)
			{
				MOTOR2_IN1 = motor_phase_table[motor2_phase_idx][0];
				MOTOR2_IN2 = motor_phase_table[motor2_phase_idx][1];
				MOTOR2_IN3 = motor_phase_table[motor2_phase_idx][2];
				MOTOR2_IN4 = motor_phase_table[motor2_phase_idx][3];
				motor2_phase_idx++;
				if(motor2_phase_idx >=8) motor2_phase_idx=0;
			}
			else
			{
				MOTOR2_IN1 = motor_phase_table[motor2_phase_idx][0];
				MOTOR2_IN2 = motor_phase_table[motor2_phase_idx][1];
				MOTOR2_IN3 = motor_phase_table[motor2_phase_idx][2];
				MOTOR2_IN4 = motor_phase_table[motor2_phase_idx][3];
				motor2_phase_idx--;
				if(motor2_phase_idx ==0) motor2_phase_idx=7;
			}
			motor2_remain_step--;
		}
		else
		{
			MOTOR2_IN1=0;MOTOR2_IN2=0;MOTOR2_IN3=0;MOTOR2_IN4=0;
		}
	}
}

