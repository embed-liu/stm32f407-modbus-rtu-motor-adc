# STM32F407‑Modbus‑RTU‑Motor‑ADC
> 硬件平台：STM32F407ZGT6(普中战神开发板)
> 开发环境：Keil MDK‑ARM
> 通信：RS485总线 Modbus‑RTU 从站

## 📋 项目简介
基于FreeModbus实现Modbus‑RTU从站。
通过RS485上位机（Modbus Poll）读写保持寄存器；实现4路ADC采集、两台28BYJ‑48步进电机控制、TFT‑LCD屏幕显示。

## 引脚定义
- USART2（RS485 Modbus通信）：PA2(TX)，PA3(RX)
- RS485 DE/RE 控制引脚：PG8 （高电平发送，低电平接收）
- ADC采集通道：PA4、PA5、PA6、PA7
- 步进电机1：PC0、PC1、PC2、PC3
- 步进电机2：PC4、PC5、PC6、PC7
- TFTLCD屏幕：沿用开发板硬件引脚（未改动LCD底层驱动）
- 按键、LED：开发板预留IO，用于调试

## ✨ 功能
1. Modbus‑RTU从站参数：从站地址 `0x01`，波特率9600，8‑N‑1
2. 支持功能码：**03读保持寄存器、06写单个保持寄存器**
3. 外设：4通道ADC采集、双路步进电机驱动、TFT‑LCD显示、RS485通信

### Modbus Poll寄存器映射
| Modbus寄存器地址 | 功能说明 | 读写属性 |
|:---:|---|:---:|
| 0‑3 | 4路ADC采样数值 | R |
| 4‑5 | 两台电机剩余步数 | R |
| 9 | 电机1目标步数 | R/W |
| 10 | 电机2目标步数 | R/W |
| 11 | M1转动方向(0/1) | R/W |
| 12 | M2转动方向(0/1) | R/W |

## 📂 工程目录结构（实际本地目录）
```
├─ DebugConfig     // Keil调试配置
├─ Driver          // 外设驱动：ADC、步进电机、RS485、LCD、按键等
├─ FreeModbus      // FreeModbus协议栈源码
├─ image           // 图片资源
├─ Library         // STM32F4标准库
├─ Start           // 启动文件
├─ System          // SysTick、延时、系统底层封装
└─ User            // main.c
```

## 🛠 使用说明
1. Keil MDK打开工程编译下载到开发板
2. USB‑RS485转换器连接开发板485接口
3. Modbus Poll配置：Slave ID=1，9600‑8‑N‑1，即可读写寄存器

> ⚠️注意：`eMBPoll()` 在主循环while(1)轮询调用，**禁止放在中断内部**。

## 📌 局限说明（调试备注）
- 当前未实现寄存器越界防护；
- 电机运行状态没有忙锁，高速连续写指令可能发生指令覆盖；

## 📝迭代计划
- ✅v1.0 当前仓库：裸机Modbus‑RTU从站，电机+ADC采集
