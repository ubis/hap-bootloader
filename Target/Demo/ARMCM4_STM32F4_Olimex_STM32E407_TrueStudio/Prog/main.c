/************************************************************************************//**
* \file         Demo\ARMCM4_STM32F4_Olimex_STM32E407_TrueStudio\Prog\main.c
* \brief        Demo program application source file.
* \ingroup      Prog_ARMCM4_STM32F4_Olimex_STM32E407_TrueStudio
* \internal
*----------------------------------------------------------------------------------------
*                          C O P Y R I G H T
*----------------------------------------------------------------------------------------
*   Copyright (c) 2018  by Feaser    http://www.feaser.com    All rights reserved
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
#include "header.h"                                    /* generic header               */


/****************************************************************************************
* Function prototypes
****************************************************************************************/
static void Init(void);


/************************************************************************************//**
** \brief     This is the entry point for the bootloader application and is called 
**            by the reset interrupt vector after the C-startup routines executed.
** \return    Program return code.
**
****************************************************************************************/
int main(void)
{
  /* initialize the microcontroller */
  Init();
  /* initialize the network application */
  NetInit();
  /* initialize the bootloader interface */
  BootComInit();

  /* start the infinite program loop */
  while (1)
  {
    /* toggle LED with a fixed frequency */
    LedToggle();
    /* run the network task */ 
    NetTask();
    /* check for bootloader activation request */
    BootComCheckActivationRequest();
  }

  /* Program should never get here. */
  return 0;
} /*** end of main ***/


/************************************************************************************//**
** \brief     Initializes the microcontroller.
** \return    none.
**
****************************************************************************************/
static void Init(void)
{
  /* initialize the system and its clocks */
  SystemInit();
  /* init the led driver */
  LedInit();
  /* init the timer driver */
  TimerInit();
} /*** end of Init ***/


/*********************************** end of main.c *************************************/
