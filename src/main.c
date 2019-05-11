/*
 *		      HAP-Bootoader, based on OpenBLT
 *
 *	@author		Jokubas Maciulaitis (ubis)
 *	@file		main.c
 *	@date		2019 04 19
 */

#include <string.h>
#include <boot.h>
#include <can.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/crc.h>

#include "protodef.h"

// unique stm32f1 id 96bit
#define U_ID_1 (*((unsigned int *) 0x1FFFF7E8))

static header_t g_header;
static unsigned short g_address;

static void cpu_init(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_USART1);
	rcc_periph_clock_enable(RCC_CAN1);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);

	// UART TX
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO9);
	// UART RX
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_INPUT_FLOAT, GPIO10);

	// CAN RX
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_INPUT_FLOAT, GPIO11);

	// CAN TX
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO12);

	// Status LED
	gpio_set_mode(SYSTEM_STATUS_LED_PORT, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, SYSTEM_STATUS_LED_PIN);

	// Error LED
	gpio_set_mode(SYSTEM_ERROR_LED_PORT, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, SYSTEM_ERROR_LED_PIN);

	// Turn on both LEDs
	gpio_set(SYSTEM_STATUS_LED_PORT, SYSTEM_STATUS_LED_PIN);
	gpio_set(SYSTEM_ERROR_LED_PORT, SYSTEM_ERROR_LED_PIN);

	// Setup USART
	usart_set_baudrate(USART1, BOOT_CONSOLE_UART_BAUDRATE);
	usart_set_databits(USART1, BOOT_CONSOLE_UART_DATA_BITS);
	usart_set_stopbits(USART1, BOOT_CONSOLE_UART_STOP_BITS);
	usart_set_mode(USART1, USART_MODE_TX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

	usart_enable(USART1);
}

unsigned short calc_crc16(const unsigned char *addr, size_t len)
{
	const unsigned short mask = 0x8000;
	const unsigned short poly = 0x1021;
	unsigned short crc = 0xFFFF;
	char i;

	while (len--) {
		crc ^= (*addr++) << 8;
		for (i = 0; i < 8; ++i) {
			if ((crc & mask) != 0) {
				crc = (crc << 1) ^ poly;
			} else {
				crc = (crc << 1);
			}
		}
	}

	return crc;
}

void CpuStartUserProgramStateHook(unsigned int state)
{
	/* */
}

unsigned char CpuUserProgramStartHook(void)
{
	// Turn off both LEDs
	gpio_clear(SYSTEM_STATUS_LED_PORT, SYSTEM_STATUS_LED_PIN);
	gpio_clear(SYSTEM_ERROR_LED_PORT, SYSTEM_ERROR_LED_PIN);

	// send prepare run application
	g_header.command = COMMAND_BOOT_PREPARE_LAUNCH;
	CanSetTxMsgId(g_header.id);
	CanTransmitPacket(NULL, 0);

	// restore header
	g_header.command = COMMAND_BOOT_PERFORM;
	CanSetTxMsgId(g_header.id);

	return 1;
}

void CopInitHook(void)
{
	/* */
}

void CopServiceHook(void)
{
	static unsigned char ledOn = 0;
	static unsigned int nextBlinkEvent = 0;

	// check for blink event
	if (TimerGet() < nextBlinkEvent) {
		return;
	}

	// toggle the LED state
	if (!ledOn) {
		ledOn = 1;
		gpio_set(SYSTEM_STATUS_LED_PORT, SYSTEM_STATUS_LED_PIN);
		gpio_clear(SYSTEM_ERROR_LED_PORT, SYSTEM_ERROR_LED_PIN);
	} else {
		ledOn = 0;
		gpio_clear(SYSTEM_STATUS_LED_PORT, SYSTEM_STATUS_LED_PIN);
		gpio_set(SYSTEM_ERROR_LED_PORT, SYSTEM_ERROR_LED_PIN);
	}

	nextBlinkEvent = TimerGet() + SYSTEM_LED_BLINK_INTERVAL;
}

int main(void)
{
	const size_t id_len = 12;
	unsigned char id_arr[id_len];

	// configure clock and peripherals
	cpu_init();

	// generate address from uid
	memcpy(id_arr, &U_ID_1, 12);
	g_address = calc_crc16(&id_arr[0], id_len);
	memcpy(id_arr, &g_address, sizeof(g_address));

	// set can rx header
	g_header.mode = MODE_BOOT;
	g_header.reserved = 0;
	g_header.type = 0;
	g_header.command = COMMAND_BOOT_PERFORM;
	g_header.group = g_address & 0xFF;
	g_header.address = (g_address & 0xFF00) >> 8;
	g_header.identifier = ID_29_BIT;
	CanSetRxMsgId(g_header.id);

	// set can tx header
	g_header.command = COMMAND_BOOT_INIT;
	g_header.group = 1;
	g_header.address = 1;
	CanSetTxMsgId(g_header.id);

	// initialize the bootloader
	BootInit();

	ee_printf("\r\n");

	// turn off both LEDs
	gpio_clear(SYSTEM_STATUS_LED_PORT, SYSTEM_STATUS_LED_PIN);
	gpio_clear(SYSTEM_ERROR_LED_PORT, SYSTEM_ERROR_LED_PIN);

	ee_printf("Device address: 0x%02X\r\n", g_address);
	ee_printf("Sending boot init command...\r\n");
	CanSetTxMsgId(g_header.id);
	CanTransmitPacket(&id_arr[0], sizeof(g_address));

	// restore header
	g_header.command = COMMAND_BOOT_PERFORM;
	CanSetTxMsgId(g_header.id);

	ee_printf("\r\n");
	ee_printf("Autoboot in %d seconds...\r\n",
		  BOOT_BACKDOOR_ENTRY_TIMEOUT_MS / 1000);

	while (1) {
		// run boot task
		BootTask();
	}

	return 0;
}
