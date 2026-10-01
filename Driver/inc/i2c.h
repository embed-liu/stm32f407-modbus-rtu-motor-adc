#ifndef __I2C_H
#define __I2C_H
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

#define I2C_SCL_PIN    GPIO_Pin_8
#define I2C_SDA_PIN    GPIO_Pin_9
#define I2C_PORT       GPIOB
#define RCC_I2C_PORT   RCC_AHB1Periph_GPIOB

#define AT24C02_ADDR   0xA0  // 器件地址，A0/A1/A2全部接地

void I2C_GPIO_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_SendByte(uint8_t dat);
uint8_t I2C_ReadByte(uint8_t ack);
uint8_t I2C_WaitAck(void);
void I2C_SendAck(uint8_t ack);

// AT24C02对外接口
uint8_t AT24C02_WriteByte(uint8_t addr,uint8_t data);
uint8_t AT24C02_ReadByte(uint8_t addr,uint8_t *pdata);
uint8_t AT24C02_WriteBuf(uint8_t start_addr,uint8_t *buf,uint8_t len);
uint8_t AT24C02_ReadBuf(uint8_t start_addr,uint8_t *buf,uint8_t len);

#endif
