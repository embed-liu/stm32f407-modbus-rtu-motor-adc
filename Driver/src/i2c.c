#include "i2c.h"
#include "SysTick.h"

static void SCL_H(void){GPIO_SetBits(I2C_PORT,I2C_SCL_PIN);}
static void SCL_L(void){GPIO_ResetBits(I2C_PORT,I2C_SCL_PIN);}
static void SDA_H(void){GPIO_SetBits(I2C_PORT,I2C_SDA_PIN);}
static void SDA_L(void){GPIO_ResetBits(I2C_PORT,I2C_SDA_PIN);}
static void SDA_IN(void){GPIO_InitTypeDef gpio_cfg;gpio_cfg.GPIO_Pin=I2C_SDA_PIN;gpio_cfg.GPIO_Mode=GPIO_Mode_IN;gpio_cfg.GPIO_PuPd=GPIO_PuPd_UP;GPIO_Init(I2C_PORT,&gpio_cfg);}
static void SDA_OUT(void){GPIO_InitTypeDef gpio_cfg;gpio_cfg.GPIO_Pin=I2C_SDA_PIN;gpio_cfg.GPIO_Mode=GPIO_Mode_OUT;gpio_cfg.GPIO_OType=GPIO_OType_OD;gpio_cfg.GPIO_Speed=GPIO_Speed_100MHz;gpio_cfg.GPIO_PuPd=GPIO_PuPd_UP;GPIO_Init(I2C_PORT,&gpio_cfg);}

void I2C_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio_cfg;
    RCC_AHB1PeriphClockCmd(RCC_I2C_PORT,ENABLE);

    gpio_cfg.GPIO_Pin = I2C_SCL_PIN;
    gpio_cfg.GPIO_Mode=GPIO_Mode_OUT;
    gpio_cfg.GPIO_OType=GPIO_OType_OD;
    gpio_cfg.GPIO_Speed=GPIO_Speed_100MHz;
    gpio_cfg.GPIO_PuPd=GPIO_PuPd_UP;
    GPIO_Init(I2C_PORT,&gpio_cfg);

    gpio_cfg.GPIO_Pin = I2C_SDA_PIN;
    gpio_cfg.GPIO_Mode=GPIO_Mode_OUT;
    gpio_cfg.GPIO_OType=GPIO_OType_OD;
    gpio_cfg.GPIO_Speed=GPIO_Speed_100MHz;
    gpio_cfg.GPIO_PuPd=GPIO_PuPd_UP;
    GPIO_Init(I2C_PORT,&gpio_cfg);

    SCL_H();
    SDA_H();
}

void I2C_Start(void)
{
    SDA_OUT();
    SDA_H();SCL_H();delay_us(5);
    SDA_L();delay_us(5);
    SCL_L();
}

void I2C_Stop(void)
{
    SDA_OUT();
    SDA_L();SCL_H();delay_us(5);
    SDA_H();delay_us(5);
}

void I2C_SendByte(uint8_t dat)
{
    uint8_t i;
    SDA_OUT();
    for(i=0;i<8;i++)
    {
        SCL_L();
        if(dat&0x80) SDA_H();else SDA_L();
        dat<<=1;
        delay_us(2);
        SCL_H();delay_us(2);
    }
    SCL_L();
}

uint8_t I2C_WaitAck(void)
{
    uint8_t ack=1;
    SDA_IN();
    SCL_H();delay_us(2);
    if(GPIO_ReadInputDataBit(I2C_PORT,I2C_SDA_PIN)==0) ack=0;
    SCL_L();
    SDA_OUT();
    return ack;
}

uint8_t I2C_ReadByte(uint8_t ack)
{
    uint8_t i,dat=0;
    SDA_IN();
    for(i=0;i<8;i++)
    {
        SCL_L();delay_us(2);
        SCL_H();delay_us(2);
        dat<<=1;
        if(GPIO_ReadInputDataBit(I2C_PORT,I2C_SDA_PIN)) dat|=0x01;
    }
    I2C_SendAck(ack);
    return dat;
}

void I2C_SendAck(uint8_t ack)
{
    SDA_OUT();
    SCL_L();
    if(ack) SDA_H();else SDA_L();
    delay_us(2);
    SCL_H();delay_us(2);
    SCL_L();
    SDA_H();
}

/**
 * @brief AT24C02单字节写入
 * @param addr: EEPROM存储地址 0~255
 * @param data: 待写入字节
 * @retval 0成功，非0失败
 */
uint8_t AT24C02_WriteByte(uint8_t addr,uint8_t data)
{
    I2C_Start();
    I2C_SendByte(AT24C02_ADDR);
    if(I2C_WaitAck()){I2C_Stop();return 1;}
    I2C_SendByte(addr);
    if(I2C_WaitAck()){I2C_Stop();return 2;}
    I2C_SendByte(data);
    if(I2C_WaitAck()){I2C_Stop();return 3;}
    I2C_Stop();
    delay_ms(5); // AT24C02内部烧写等待，必须！
    return 0;
}

/**
 * @brief AT24C02单字节读取
 * @param addr: EEPROM地址
 * @param pdata: 读出数据指针
 * @retval 0成功
 */
uint8_t AT24C02_ReadByte(uint8_t addr,uint8_t *pdata)
{
    I2C_Start();
    I2C_SendByte(AT24C02_ADDR);
    if(I2C_WaitAck()){I2C_Stop();return 1;}
    I2C_SendByte(addr);
    if(I2C_WaitAck()){I2C_Stop();return 2;}
    I2C_Start();
    I2C_SendByte(AT24C02_ADDR | 0x01);
    if(I2C_WaitAck()){I2C_Stop();return 3;}
    *pdata = I2C_ReadByte(1); // 最后一字节NACK
    I2C_Stop();
    return 0;
}

/**
 * @brief AT24C02多字节页写，一次最多8字节，不能跨页
 * @param start_addr 起始地址
 * @param buf 数据源
 * @param len 长度(<=8)
 * @retval 0成功
 */
uint8_t AT24C02_WriteBuf(uint8_t start_addr,uint8_t *buf,uint8_t len)
{
    uint8_t i;
    if(len>8) return 1;
    I2C_Start();
    I2C_SendByte(AT24C02_ADDR);
    if(I2C_WaitAck()){I2C_Stop();return 2;}
    I2C_SendByte(start_addr);
    if(I2C_WaitAck()){I2C_Stop();return 3;}
    for(i=0;i<len;i++)
    {
        I2C_SendByte(buf[i]);
        if(I2C_WaitAck()){I2C_Stop();return 4;}
    }
    I2C_Stop();
    delay_ms(5);
    return 0;
}

/**
 * @brief AT24C02连续多字节读，无8字节限制
 */
uint8_t AT24C02_ReadBuf(uint8_t start_addr,uint8_t *buf,uint8_t len)
{
    uint8_t i;
    I2C_Start();
    I2C_SendByte(AT24C02_ADDR);
    if(I2C_WaitAck()){I2C_Stop();return 1;}
    I2C_SendByte(start_addr);
    if(I2C_WaitAck()){I2C_Stop();return 2;}
    I2C_Start();
    I2C_SendByte(AT24C02_ADDR | 0x01);
    if(I2C_WaitAck()){I2C_Stop();return 3;}
    for(i=0;i<len;i++)
    {
        if(i==len-1) buf[i]=I2C_ReadByte(1); //最后NACK
        else buf[i]=I2C_ReadByte(0);
    }
    I2C_Stop();
    return 0;
}
