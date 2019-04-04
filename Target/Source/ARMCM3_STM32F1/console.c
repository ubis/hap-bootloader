/************************************************************************************//**
* \file         Source\ARMCM3_STM32F1\console.c
* \brief        Bootloader Console UART communication interface source file.
* \ingroup      Target_ARMCM3_STM32F1
* \internal
*----------------------------------------------------------------------------------------
*                          C O P Y R I G H T
*----------------------------------------------------------------------------------------
*   Copyright (c) 2011  by Feaser    http://www.feaser.com    All rights reserved
*
*----------------------------------------------------------------------------------------
*                            L I C E N S E
*----------------------------------------------------------------------------------------
* This file is part of OpenBLT. OpenBLT is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published by the Free
* Software Foundation, either version 3 of the License, or (at your option) any later
* version.
*
* OpenBLT is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE. See the GNU General Public License for more details.
*
* You have received a copy of the GNU General Public License along with OpenBLT. It
* should be located in ".\Doc\license.html". If not, contact Feaser to obtain a copy.
*
* \endinternal
****************************************************************************************/

/****************************************************************************************
* Include files
****************************************************************************************/
#include <stdlib.h>
#include "boot.h"                                /* bootloader generic header          */


#if (BOOT_CONSOLE_UART_ENABLE > 0)
/****************************************************************************************
* Type definitions
****************************************************************************************/
/** \brief UART register layout. */
typedef struct
{
  volatile blt_int16u SR;                           /**< status register               */
  blt_int16u          RESERVED0;
  volatile blt_int16u DR;                           /**< data register                 */
  blt_int16u          RESERVED1;
  volatile blt_int16u BRR;                          /**< baudrate register             */
  blt_int16u          RESERVED2;
  volatile blt_int16u CR1;                          /**< control register 1            */
  blt_int16u          RESERVED3;
  volatile blt_int16u CR2;                          /**< control register 2            */
  blt_int16u          RESERVED4;
  volatile blt_int16u CR3;                          /**< control register 3            */
  blt_int16u          RESERVED5;
  volatile blt_int16u GTPR;                         /**< guard time and prescale reg.  */
  blt_int16u          RESERVED6;
} tUartRegs;                                        /**< UART register layout type     */


/****************************************************************************************
* Register definitions
****************************************************************************************/
#if (BOOT_CONSOLE_COM_UART_CHANNEL_INDEX == 0)
/** \brief Set UART base address to USART1. */
#define UARTx          ((tUartRegs *) (blt_int32u)0x40013800)
#elif (BOOT_CONSOLE_COM_UART_CHANNEL_INDEX == 1)
/** \brief Set UART base address to USART2. */
#define UARTx          ((tUartRegs *) (blt_int32u)0x40004400)
#else
/** \brief Set UART base address to USART1 by default. */
#define UARTx          ((tUartRegs *) (blt_int32u)0x40013800)
#endif

#define USART_SR_TXE (1 << 7)

#define FLASH_SIZE *((blt_int16u *)(0x1FFFF7E0))


/****************************************************************************************
* Function prototypes
****************************************************************************************/
static void send_char(const blt_char ch);
static void send_num(blt_int32s num);


/************************************************************************************//**
** \brief     Initializes the Console UART communication interface.
** \return    none.
**
****************************************************************************************/
void ConsoleInit(void)
{
    blt_char buffer[8] = { 0 };

    ConsoleSend("OpenBLT ");

    // send bootloader version info
    ConsoleSend(BOOT_VERSION_CORE_DATE);
    ConsoleSend("-v");
    send_num(BOOT_VERSION_CORE_MAIN);
    send_num(BOOT_VERSION_CORE_MINOR);
    send_num(BOOT_VERSION_CORE_PATCH);

    // send build date
    ConsoleSend(" (");
    ConsoleSend(__DATE__);
    ConsoleSend(" - ");
    ConsoleSend(__TIME__);
    ConsoleSend(")\r\n");

    // send cpu info
    ConsoleSend("CPU:\tSTM32F103C8T6 at 72MHz\r\n"); // fix me

    // send ram info
    ConsoleSend("RAM:\t20 KiB\r\n"); // fix me

    // send flash info
    ConsoleSend("Flash:\t");
    itoa(FLASH_SIZE, buffer, 10);
    ConsoleSend(buffer);
    ConsoleSend(" KiB\r\n");

    ConsoleSend("\r\n");

#if (BOOT_COM_CAN_ENABLE > 0)
    ConsoleSend("CAN at ");
    itoa(BOOT_COM_CAN_BAUDRATE, buffer, 10);
    ConsoleSend(buffer);
    ConsoleSend(" kbit/s initialized, ready\r\n");
#else
    ConsoleSend("Warning: no communication enabled!\r\n");
#endif
} /*** end of ConsoleInit ***/


/************************************************************************************//**
** \brief     Sends an array of characters to Console Through UART.
** \param     data Pointer to byte array with data that it to be sent.
** \return    none.
**
****************************************************************************************/
void ConsoleSend(const blt_char *data)
{
    while (*data != 0) {
        send_char(*data);
        data++;
    }
} /*** end of ConsoleSend ***/


/************************************************************************************//**
** \brief     Sends a single character to Console through UART.
** \param     ch Character to be sent.
** \return    none.
**
****************************************************************************************/
static void send_char(const blt_char ch)
{
    while (!(UARTx->SR & USART_SR_TXE));
    UARTx->DR = ch;
} /*** end of send_char ***/

/************************************************************************************//**
** \brief     Sends a single number to Console through UART and appends leading zero.
** \param     num Number to be sent.
** \return    none.
**
****************************************************************************************/
static void send_num(blt_int32s num)
{
    if (num < 10) {
        send_char('0');
    }

    send_char(num + '0');
} /*** end of send_num ***/

#endif /* BOOT_CONSOLE_UART_ENABLE > 0 */


/*********************************** end of console.c *************************************/
