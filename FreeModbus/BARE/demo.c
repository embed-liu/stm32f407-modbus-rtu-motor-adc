/*
 * FreeModbus Libary: BARE Demo Application
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"



/*----------------------------------------------------------------
名称：         线圈
存储区标识：   0XXXX
类型：         位
读写：         读/写
存储单元地址：(00001~0XXXX).XXXX:与设备相关
功能码：       读取线圈状态 (0x01)，写单个线圈 (0x05)，写多个线圈 (0x0F)
用途：         DO继电器
//-----------------------------------------------------------------*/
#define REG_COILS_START     0x0001
#define REG_COILS_SIZE      16
unsigned char       ucRegCoilsBuf[REG_COILS_SIZE / 8]={0X00,0X00};

/*----------------------------------------------------------------
名称：         输入线圈
存储区标识：   1XXXX
类型：         位
读写：         只读
存储单元地址： (10001~1XXXX).XXXX:与设备相关
功能码：       读输入状态 (0x02)
用途：         DI继电器
//-----------------------------------------------------------------*/
/* ----------------------- Defines ------------------------------------------*/
#define REG_DISC_START     0x0001
#define REG_DISC_SIZE      16
unsigned char ucRegDiscBuf[REG_DISC_SIZE / 8] = { 0x0F, 0XF0 };

/*----------------------------------------------------------------
名称：         输入寄存器
存储区标识：   3XXXX
类型：         字
读写：         只读
存储单元地址：(30001~3XXXX).XXXX:与设备相关
功能码：       读输入寄存器 (0x04)
用途：         模拟量输入
//-----------------------------------------------------------------*/
#define REG_INPUT_START 0x0001
#define REG_INPUT_NREGS 8
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS]=
{0x1111,0x2222,0x3333,0x4444,0x5555,0x6666,0x7777,0x8888};


/*----------------------------------------------------------------
名称：         保持/输出寄存器
存储区标识：   4XXXX
类型：         字
读写：         读/写
存储单元地址：(40001~4XXXX).XXXX:与设备相关
功能码：       读保持寄存器 (0x03)，写单个寄存器 (0x06)，写多个寄存器 (0x10)
用途：         模拟量输出
//-----------------------------------------------------------------*/
#define REG_HOLDING_START 0x0001 //保持寄存器起始地址 
#define REG_HOLDING_NREGS 8    //保持寄存器数量
USHORT   usRegHoldingStart = REG_HOLDING_START;
USHORT   usRegHoldingBuf[REG_HOLDING_NREGS]=
{0X1100,0X2200,0X3300,0X4400,0X5500,0X6600,0X7700,0X8800}; 


/*===================================================================
function name: eMBRegInputCB()
Description:   0x04 输入寄存器处理函数，将数据放入BUF 
Attention：
Input Parameters: 
Returncode：
====================================================================*/
eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

/*===================================================================
function name: eMBRegHoldingCB()
Description:   0x03，0x06.0x10 保持寄存器处理函数，将数据放入BUF 
Attention：
Input Parameters: 
Returncode：
====================================================================*/
eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                 eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if((usAddress>=REG_HOLDING_START) && (usAddress+usNRegs<=REG_HOLDING_START+REG_HOLDING_NREGS) )
    {
        iRegIndex = ( int )( usAddress - usRegHoldingStart );
        switch ( eMode )
        {
        case MB_REG_READ:
            while( usNRegs > 0 )
            {
				*pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] >> 8 );
				*pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] & 0xFF );
				iRegIndex++;

				usNRegs--;
            }
            break;

        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
				usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
			break;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/*===================================================================
function name: eMBRegCoilsCB()
Description:  0x01 ,0x05,0x0F 线圈处理函数，将数据放入BUF
Attention：0x01读输出（DO），0x05,0x0f强制单个，多个线圈（DO）
Input Parameters: 
Returncode：
====================================================================*/
eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
               eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    
    int             iNCoils = ( int )usNCoils;
    unsigned short  usBitOffset;

    /* Check if we have registers mapped at this block. */
    if( ( usAddress >= REG_COILS_START ) &&( usAddress + usNCoils <= REG_COILS_START + REG_COILS_SIZE ) )
    {
        usBitOffset = ( unsigned short )( usAddress - REG_COILS_START );
        switch ( eMode )
        {
                /* Read current values and pass to protocol stack. */
            case MB_REG_READ:
                while( iNCoils > 0 )
                {
                    *pucRegBuffer++ = xMBUtilGetBits( ucRegCoilsBuf, usBitOffset, ( unsigned char )( iNCoils > 8 ? 8 :iNCoils ) );
                    iNCoils -= 8;
                    usBitOffset += 8;
                }
                break;

                /* Update current register values. */
            case MB_REG_WRITE:
				while( iNCoils > 0 )
                {
                    xMBUtilSetBits( ucRegCoilsBuf, usBitOffset,( unsigned char )( iNCoils > 8 ? 8 : iNCoils ), *pucRegBuffer++ );
                    iNCoils -= 8;
                    usBitOffset += 8;
                }
                break;
        }

    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/*===================================================================
function name: eMBRegDiscreteCB()
Description:  0x02 读输入状态/读取输入状态处理函数，将数据放入BUF
Attention：
Input Parameters: 
Returncode：
====================================================================*/
eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    short           iNDiscrete = ( short )usNDiscrete;
    unsigned short  usBitOffset;

    /* Check if we have registers mapped at this block. */
    if( (usAddress >= REG_DISC_START) &&( usAddress + usNDiscrete<= REG_DISC_START + REG_DISC_SIZE ))
    {
        usBitOffset = (unsigned short)( usAddress - REG_DISC_START );
        while( iNDiscrete > 0 )
        {
            *pucRegBuffer++ =xMBUtilGetBits(ucRegDiscBuf, usBitOffset, (unsigned char)( iNDiscrete >8?8: iNDiscrete ) );
            iNDiscrete -= 8;
            usBitOffset+=8;
        }
    }
    else
    {
        eStatus = MB_ENOREG;//illegal register address
    }
    return eStatus;
}
