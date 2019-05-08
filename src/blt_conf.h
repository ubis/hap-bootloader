/*
 *		      HAP-Bootoader, based on OpenBLT
 *
 *	@author		Jokubas Maciulaitis (ubis)
 *	@file		blt_conf.h
 *	@date		2019 04 22
 */

#ifndef BLT_CONF_H
#define BLT_CONF_H

/*******************************************************************************
*   C P U   D R I V E R   C O N F I G U R A T I O N
*******************************************************************************/
/* To properly initialize the baudrate clocks of the communication interface,
 * typically the speed of the crystal oscillator and/or the speed at which the
 * system runs is needed. Set these through configurables
 * BOOT_CPU_XTAL_SPEED_KHZ and BOOT_CPU_SYSTEM_SPEED_KHZ, respectively.
 * To enable data exchange with the host that is not dependent on the targets
 * architecture, the byte ordering needs to be known.
 * Setting BOOT_CPU_BYTE_ORDER_MOTOROLA to 1 selects big endian mode and 0
 * selects little endian mode.
 *
 * Set BOOT_CPU_USER_PROGRAM_START_HOOK to 1 if you would like a hook function
 * to be called the moment the user program is about to be started. This could
 * be used to de-initialize application specific parts, for example to stop
 * blinking an LED, etc.
 */

// Frequency of the external crystal oscillator
#define BOOT_CPU_XTAL_SPEED_KHZ 	 (8000)
// Desired system speed
#define BOOT_CPU_SYSTEM_SPEED_KHZ 	 (72000)
// Motorola or Intel style byte ordering
#define BOOT_CPU_BYTE_ORDER_MOTOROLA 	 (0)
// Enable/disable hook function call right before user program start
#define BOOT_CPU_USER_PROGRAM_START_HOOK (1)

/*******************************************************************************
*   C O M M U N I C A T I O N   I N T E R F A C E   C O N F I G U R A T I O N
*******************************************************************************/
/* The CAN communication interface is selected by setting the
 * BOOT_COM_CAN_ENABLE configurable to 1. Configurable BOOT_COM_CAN_BAUDRATE
 * selects the communication speed in bits/second. Two CAN messages are
 * reserved for communication with the host. The message identifier for
 * sending data from the target to the host is configured with
 * BOOT_COM_CAN_TXMSG_ID. The one for receiving data from the host is
 * configured with BOOT_COM_CAN_RXMSG_ID. The maximum amount of data bytes in
 * a message for data transmission and reception is set through
 * BOOT_COM_CAN_TX_MAX_DATA and BOOT_COM_CAN_RX_MAX_DATA, respectively. It is
 * common for a microcontroller to have more than 1 CAN controller on board.
 * The zero-based BOOT_COM_CAN_CHANNEL_INDEX selects the CAN controller channel.
 */

// Enable/disable CAN transport layer
#define BOOT_COM_CAN_ENABLE 		(1)
// Configure the desired CAN baudrate
#define BOOT_COM_CAN_BAUDRATE 		(125000)
// Configure CAN message ID target->host
#define BOOT_COM_CAN_TX_MSG_ID 		(0x7E1)
// Configure number of bytes in the target->host CAN message
#define BOOT_COM_CAN_TX_MAX_DATA 	(8)
// Configure CAN message ID host->target
#define BOOT_COM_CAN_RX_MSG_ID 		(0x667)
// Configure number of bytes in the host->target CAN message
#define BOOT_COM_CAN_RX_MAX_DATA 	(8)
// Select the desired CAN peripheral as a zero based index
#define BOOT_COM_CAN_CHANNEL_INDEX 	(0)

/*******************************************************************************
*   C O N S O L E   I N T E R F A C E   C O N F I G U R A T I O N
*******************************************************************************/
/* The Console interface is selected by setting the BOOT_CONSOLE_UART_ENABLE
 * configurable to 1. Configurable BOOT_CONSOLE_UART_BAUDRATE selects the
 * console communication speed in bits/second. It is common for a
 * microcontroller to have more than 1 UART controller on board. The zero-based
 * BOOT_CONSOLE_COM_UART_CHANNEL_INDEX selects the UART controller channel.
 */

// Enable/disable the UART console interface
#define BOOT_CONSOLE_UART_ENABLE 		1
// Configure the desired UART baudrate
#define BOOT_CONSOLE_UART_BAUDRATE 		(115200)
// Configure the desired UART data bits
#define BOOT_CONSOLE_UART_DATA_BITS		(8)
// Configure the desired UART stop bits
#define BOOT_CONSOLE_UART_STOP_BITS		(1)
// Select the desired UART peripheral as a zero based index
#define BOOT_CONSOLE_COM_UART_CHANNEL_INDEX 	0

/*******************************************************************************
*   N O N - V O L A T I L E  M E M O R Y  D R I V E R  C O N F I G U R A T I O N
*******************************************************************************/
/* The NVM driver typically supports erase and program operations of the
 * internal memory present on the microcontroller.
 * The size of the internal memory in kilobytes is specified with configurable
 * BOOT_NVM_SIZE_KB.
 */

// Configure the size of the default memory device (typically flash EEPROM)
#define BOOT_NVM_SIZE_KB (128)

/*******************************************************************************
*   F L A S H   M E M O R Y   D R I V E R   C O N F I G U R A T I O N
*******************************************************************************/

/* This microcontroller has a smaller vector table then the default STM32F1xx
 * project as assumed in the bootloader's core. This means the user program has
 * a different checksum location, because this one is added at the end of the
 * user program's vector table.
 */
#define BOOT_FLASH_VECTOR_TABLE_CS_OFFSET (0xEC)

/* Enable support for a custom flash layout table. It is located in
 * flash_layout.c. This was done because the default flashLayout[] table
 * in the bootloader's core has more flash memory reserved for the bootloader
 * than is needed for this demo.
 */
#define BOOT_FLASH_CUSTOM_LAYOUT_ENABLE (1)

/*******************************************************************************
*   W A T C H D O G   D R I V E R   C O N F I G U R A T I O N
*******************************************************************************/
/* The COP driver cannot be configured internally in the bootloader, because
 * its use and configuration is application specific. The bootloader does need
 * to service the watchdog in case it is used. When the application requires
 * the use of a watchdog, set BOOT_COP_HOOKS_ENABLE to be able to initialize
 * and service the watchdog through hook functions.
 */

// Enable/disable the hook functions for controlling the watchdog
#define BOOT_COP_HOOKS_ENABLE (1)

#endif
