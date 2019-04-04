/************************************************************************************//**
* \file         Demo\ARMCM0_XMC1_XMC1400_Boot_Kit_IAR\Prog\boot.c
* \brief        Demo program bootloader interface source file.
* \ingroup      Prog_ARMCM0_XMC1_XMC1400_Boot_Kit_IAR
* \internal
*----------------------------------------------------------------------------------------
*                          C O P Y R I G H T
*----------------------------------------------------------------------------------------
*   Copyright (c) 2016  by Feaser    http://www.feaser.com    All rights reserved
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
#include "xmc_gpio.h"                                  /* GPIO module                  */
#include "xmc_uart.h"                                  /* UART driver header           */
#include "xmc_can.h"                                   /* CAN driver header            */


/****************************************************************************************
* Function prototypes
****************************************************************************************/
#if (BOOT_COM_UART_ENABLE > 0)
static void BootComUartInit(void);
static void BootComUartCheckActivationRequest(void);
#endif
#if (BOOT_COM_CAN_ENABLE > 0)
static void BootComCanInit(void);
static void BootComCanCheckActivationRequest(void);
#endif

/************************************************************************************//**
** \brief     Initializes the communication interface.
** \return    none.
**
****************************************************************************************/
void BootComInit(void)
{
#if (BOOT_COM_UART_ENABLE > 0)
  BootComUartInit();
#endif
#if (BOOT_COM_CAN_ENABLE > 0)
  BootComCanInit();
#endif
} /*** end of BootComInit ***/


/************************************************************************************//**
** \brief     Receives the CONNECT request from the host, which indicates that the
**            bootloader should be activated and, if so, activates it.
** \return    none.
**
****************************************************************************************/
void BootComCheckActivationRequest(void)
{
#if (BOOT_COM_UART_ENABLE > 0)
  BootComUartCheckActivationRequest();
#endif
#if (BOOT_COM_CAN_ENABLE > 0)
  BootComCanCheckActivationRequest();
#endif
} /*** end of BootComCheckActivationRequest ***/


/************************************************************************************//**
** \brief     Bootloader activation function.
** \return    none.
**
****************************************************************************************/
void BootActivate(void)
{
  /* perform software reset to activate the bootoader again */
  NVIC_SystemReset();
} /*** end of BootActivate ***/


#if (BOOT_COM_UART_ENABLE > 0)
/****************************************************************************************
*     U N I V E R S A L   A S Y N C H R O N O U S   R X   T X   I N T E R F A C E
****************************************************************************************/

/****************************************************************************************
* Macro definitions
****************************************************************************************/
/** \brief Timeout time for the reception of a CTO packet. The timer is started upon
 *         reception of the first packet byte.
 */
#define UART_CTO_RX_PACKET_TIMEOUT_MS (100u)


/****************************************************************************************
* Function prototypes
****************************************************************************************/
static unsigned char UartReceiveByte(unsigned char *data);


/************************************************************************************//**
** \brief     Initializes the UART communication interface.
** \return    none.
**
****************************************************************************************/
static void BootComUartInit(void)
{
  XMC_GPIO_CONFIG_t rx_config;
  XMC_GPIO_CONFIG_t tx_config;
  XMC_UART_CH_CONFIG_t uart_config;

  /* set configuration and initialize UART channel */
  uart_config.baudrate = BOOT_COM_UART_BAUDRATE;
  uart_config.data_bits = 8;
  uart_config.frame_length = 8;
  uart_config.stop_bits = 1;
  uart_config.oversampling = 16;
  uart_config.parity_mode = XMC_USIC_CH_PARITY_MODE_NONE;
  XMC_UART_CH_Init(XMC_UART0_CH1, &uart_config);
  /* initialize UART Rx pin */
  rx_config.mode = XMC_GPIO_MODE_INPUT_TRISTATE;
  XMC_GPIO_Init(P1_3, &rx_config);
  /* initialize UART Tx pin */
  tx_config.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT7;
  XMC_GPIO_Init(P1_2, &tx_config);
  /* set input source path to DXnA to connect P1_3 to ASC Receive. note that this
   * function must be called after XMC_UART_CH_Init(), which is called when initializing
   * the bootloader core with BootInit().
  */
  XMC_UART_CH_SetInputSource(XMC_UART0_CH1, XMC_UART_CH_INPUT_RXD, USIC0_C1_DX0_P1_3);
  /* configure small transmit and receive FIFO */
  XMC_USIC_CH_TXFIFO_Configure(XMC_UART0_CH1, 16U, XMC_USIC_CH_FIFO_SIZE_16WORDS, 1U);
  XMC_USIC_CH_RXFIFO_Configure(XMC_UART0_CH1,  0U, XMC_USIC_CH_FIFO_SIZE_16WORDS, 1U);
  /* start UART */
  XMC_UART_CH_Start(XMC_UART0_CH1);
} /*** end of BootComUartInit ***/


/************************************************************************************//**
** \brief     Receives the CONNECT request from the host, which indicates that the
**            bootloader should be activated and, if so, activates it.
** \return    none.
**
****************************************************************************************/
static void BootComUartCheckActivationRequest(void)
{
  static unsigned char xcpCtoReqPacket[BOOT_COM_UART_RX_MAX_DATA+1];
  static unsigned char xcpCtoRxLength;
  static unsigned char xcpCtoRxInProgress = 0;
  static unsigned long xcpCtoRxStartTime = 0;

  /* start of cto packet received? */
  if (xcpCtoRxInProgress == 0)
  {
    /* store the message length when received */
    if (UartReceiveByte(&xcpCtoReqPacket[0]) == 1)
    {
      /* check that the length has a valid value. it should not be 0 */
      if ( (xcpCtoReqPacket[0] > 0) &&
           (xcpCtoReqPacket[0] <= BOOT_COM_UART_RX_MAX_DATA) )
      {
        /* store the start time */
        xcpCtoRxStartTime = TimerGet();
        /* indicate that a cto packet is being received */
        xcpCtoRxInProgress = 1;
        /* reset packet data count */
        xcpCtoRxLength = 0;
      }
    }
  }
  else
  {
    /* store the next packet byte */
    if (UartReceiveByte(&xcpCtoReqPacket[xcpCtoRxLength+1]) == 1)
    {
      /* increment the packet data count */
      xcpCtoRxLength++;

      /* check to see if the entire packet was received */
      if (xcpCtoRxLength == xcpCtoReqPacket[0])
      {
        /* done with cto packet reception */
        xcpCtoRxInProgress = 0;

        /* check if this was an XCP CONNECT command */
        if ((xcpCtoReqPacket[1] == 0xff) && (xcpCtoReqPacket[2] == 0x00))
        {
          /* connection request received so start the bootloader */
          BootActivate();
        }
      }
    }
    else
    {
      /* check packet reception timeout */
      if (TimerGet() > (xcpCtoRxStartTime + UART_CTO_RX_PACKET_TIMEOUT_MS))
      {
        /* cancel cto packet reception due to timeout. note that this automatically
         * discards the already received packet bytes, allowing the host to retry.
         */
        xcpCtoRxInProgress = 0;
      }
    }
  }
} /*** end of BootComUartCheckActivationRequest ***/


/************************************************************************************//**
** \brief     Receives a communication interface byte if one is present.
** \param     data Pointer to byte where the data is to be stored.
** \return    1 if a byte was received, 0 otherwise.
**
****************************************************************************************/
static unsigned char UartReceiveByte(unsigned char *data)
{
  if (XMC_USIC_CH_RXFIFO_IsEmpty(XMC_UART0_CH1) == 0)
  {
    /* retrieve and store the newly received byte */
    *data = (unsigned char)XMC_UART_CH_GetReceivedData(XMC_UART0_CH1);
    /* all done */
    return 1;
  }
  /* still here to no new byte received */
  return 0;
} /*** end of UartReceiveByte ***/
#endif /* BOOT_COM_UART_ENABLE > 0 */


#if (BOOT_COM_CAN_ENABLE > 0)
/****************************************************************************************
*        C O N T R O L L E R   A R E A   N E T W O R K   I N T E R F A C E
****************************************************************************************/

/****************************************************************************************
* Local data declarations
****************************************************************************************/
/** \brief Receive message object data structure. */
static XMC_CAN_MO_t receiveMsgObj;


/************************************************************************************//**
** \brief     Initializes the CAN communication interface.
** \return    none.
**
****************************************************************************************/
static void BootComCanInit(void)
{
  XMC_GPIO_CONFIG_t rx_can_config;
  XMC_GPIO_CONFIG_t tx_can_config;
  unsigned char byteIdx;
  unsigned long canModuleFreqHz;
  XMC_CAN_NODE_NOMINAL_BIT_TIME_CONFIG_t baud;
  unsigned long receiveId;

  /* decide on fCAN frequency. it should be in the 5-120MHz range. according to the
   * datasheet, it must be at least 12MHz if 1 node (channel) is used with up to
   * 16 message objects. This is sufficient for this CAN driver.
   */
  canModuleFreqHz = XMC_SCU_CLOCK_GetPeripheralClockFrequency();
  /* increase if too low */
  while (canModuleFreqHz < 12000000)
  {
    canModuleFreqHz *= 2;
  }
  /* decrease if too high */
  while (canModuleFreqHz > 120000000)
  {
    canModuleFreqHz /= 2;
  }

  /* configure CAN module*/
  XMC_CAN_Init(CAN, XMC_CAN_CANCLKSRC_MCLK, canModuleFreqHz);

  /* configure CAN node baudrate */
  baud.can_frequency = canModuleFreqHz;
  baud.baudrate = BOOT_COM_CAN_BAUDRATE;
  baud.sample_point = 8000;
  baud.sjw = 1,
  XMC_CAN_NODE_NominalBitTimeConfigure(CAN_NODE1, &baud);

  /* set CCE and INIT bit NCR for node configuration */
  XMC_CAN_NODE_EnableConfigurationChange(CAN_NODE1);
  XMC_CAN_NODE_SetInitBit(CAN_NODE1);

  /* configure the receive message object */
  receiveMsgObj.can_mo_ptr = CAN_MO1;
  receiveMsgObj.can_priority = XMC_CAN_ARBITRATION_MODE_IDE_DIR_BASED_PRIO_2;
  /* set the receive CAN identifier and negate the bit that configures it as a
   * 29-bit extended CAN identifier.
   */
  receiveId = BOOT_COM_CAN_RX_MSG_ID;
  receiveId &= ~0x80000000;

  if ((BOOT_COM_CAN_RX_MSG_ID & 0x80000000) == 0)
  {
    /* 11-bit standard CAN identifier */
    receiveMsgObj.can_identifier = receiveId;
    receiveMsgObj.can_id_mask = receiveId;
    receiveMsgObj.can_id_mode = XMC_CAN_FRAME_TYPE_STANDARD_11BITS;
    XMC_CAN_MO_AcceptOnlyMatchingIDE(&receiveMsgObj);
  }
  else
  {
    /* 29-bit extended CAN identifier */
    receiveMsgObj.can_identifier = receiveId;
    receiveMsgObj.can_id_mask = receiveId;
    receiveMsgObj.can_id_mode = XMC_CAN_FRAME_TYPE_EXTENDED_29BITS;
    XMC_CAN_MO_AcceptOnlyMatchingIDE(&receiveMsgObj);
  }
  receiveMsgObj.can_data_length = BOOT_COM_CAN_RX_MAX_DATA;
  for (byteIdx=0; byteIdx<receiveMsgObj.can_data_length; byteIdx++)
  {
    receiveMsgObj.can_data_byte[byteIdx] = 0;
  }
  receiveMsgObj.can_mo_type = XMC_CAN_MO_TYPE_RECMSGOBJ;
  XMC_CAN_MO_Config(&receiveMsgObj);

  /* allocate receive message object to the channel */
  XMC_CAN_AllocateMOtoNodeList(CAN, 1, 1);

  /* reset CCE and INIT bit NCR for node configuration */
  XMC_CAN_NODE_DisableConfigurationChange(CAN_NODE1);
  XMC_CAN_NODE_ResetInitBit(CAN_NODE1);

  /* configure CAN receive pin */
  rx_can_config.mode = XMC_GPIO_MODE_INPUT_TRISTATE;
  XMC_GPIO_Init(P4_8, &rx_can_config);
  /* configure CAN transmit pin */
  tx_can_config.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT9;
  XMC_GPIO_Init(P4_9, &tx_can_config);
  /* select CAN Receive Input C (N1_RXDC) to map P4_8 to CAN_NODE1 */
  XMC_CAN_NODE_EnableConfigurationChange(CAN_NODE1);
  XMC_CAN_NODE_SetReceiveInput(CAN_NODE1, XMC_CAN_NODE_RECEIVE_INPUT_RXDCC);
  XMC_CAN_NODE_DisableConfigurationChange(CAN_NODE1);
} /*** end of BootComCanInit ***/


/************************************************************************************//**
** \brief     Receives the CONNECT request from the host, which indicates that the
**            bootloader should be activated and, if so, activates it.
** \return    none.
**
****************************************************************************************/
static void BootComCanCheckActivationRequest(void)
{
  unsigned char byteIdx;
  unsigned char rxMsgData[8];
  unsigned char rxMsgReceived = 0;

  /* check if a new message was received */
  if ((XMC_CAN_MO_GetStatus(&receiveMsgObj) & XMC_CAN_MO_STATUS_RX_PENDING) != 0)
  {
    /* read out and process the newly received data */
    if (XMC_CAN_MO_ReceiveData(&receiveMsgObj) == XMC_CAN_STATUS_SUCCESS)
    {
      for (byteIdx=0; byteIdx<receiveMsgObj.can_data_length; byteIdx++)
      {
        rxMsgData[byteIdx] = receiveMsgObj.can_data_byte[byteIdx];
        /* set flag that message was received */
        rxMsgReceived = 1;
      }
    }
    /* reset the message received flag */
    XMC_CAN_MO_ResetStatus(&receiveMsgObj, XMC_CAN_MO_RESET_STATUS_RX_PENDING);
  }

  /* process received message */
  if (rxMsgReceived == 1)
  {
    /* reset flag */
    rxMsgReceived = 0;
    /* check if this was an XCP CONNECT command */
    if ((rxMsgData[0] == 0xff) && (rxMsgData[1] == 0x00))
    {
      /* connection request received so start the bootloader */
      BootActivate();
    }
  }
} /*** end of BootComCanCheckActivationRequest ***/
#endif /* BOOT_COM_CAN_ENABLE > 0 */


/*********************************** end of boot.c *************************************/
