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
#include <xcp.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/crc.h>

#include "protodef.h"

// unique stm32f1 id 96bit
#define U_ID_1 (*((unsigned int *) 0x1FFFF7E8))

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

static unsigned short calc_crc16(const unsigned char *addr, size_t len)
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

static void set_up_header(unsigned short *addr)
{
	const size_t id_len = 12;
	header_t g_header;
	unsigned char id_arr[id_len];

	// generate address from uid
	memcpy(id_arr, &U_ID_1, id_len);
	*addr = calc_crc16(&id_arr[0], id_len);

	// set up can rx header
	g_header.mode = MODE_BOOT;
	g_header.reserved = 0;
	g_header.address = *addr;
	g_header.identifier = ID_29_BIT;
	CanSetRxMsgId(g_header.id);

	// set can tx header
	g_header.address = 0x0101; // 0x01 address; 0x01 group
	CanSetTxMsgId(g_header.id);
}

static void send_boot_init(unsigned short *addr)
{
	unsigned char buffer[4];

	buffer[0] = XCP_PID_NFY;
	buffer[1] = XCP_CMD_BOOT_INIT;
	memcpy(buffer + 2, addr, sizeof(*addr));

	CanTransmitPacket(&buffer[0], sizeof(buffer) / sizeof(buffer[0]));
}

void CpuStartUserProgramStateHook(unsigned int state)
{
	unsigned char buffer[3];

	buffer[0] = XCP_PID_NFY;
	buffer[1] = XCP_CMD_BOOT_FAIL;
	buffer[2] = BOOT_FAIL_START;

	switch (state) {
	case BLT_STATE_CHECKSUM_FAIL:
		buffer[2] = BOOT_FAIL_CHECKSUM;
		break;
	case BLT_STATE_START_HOOK_FAIL:
		buffer[2] = BOOT_FAIL_PRE_START;
		break;
	}

	// send boot fail notification
	CanTransmitPacket(&buffer[0], sizeof(buffer) / sizeof(buffer[0]));
}

unsigned char CpuUserProgramStartHook(void)
{
	unsigned char buffer[2];

	// Turn off both LEDs
	gpio_clear(SYSTEM_STATUS_LED_PORT, SYSTEM_STATUS_LED_PIN);
	gpio_clear(SYSTEM_ERROR_LED_PORT, SYSTEM_ERROR_LED_PIN);

	buffer[0] = XCP_PID_NFY;
	buffer[1] = XCP_CMD_BOOT_PREPARE;

	// send prepare run application
	CanTransmitPacket(&buffer[0], sizeof(buffer) / sizeof(buffer[0]));
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
	unsigned short dev_addr;

	// configure clock and peripherals
	cpu_init();

	// set up CAN tx/rx header
	set_up_header(&dev_addr);

	// initialize the bootloader
	BootInit();

	ee_printf("\r\n");

	// turn off both LEDs
	gpio_clear(SYSTEM_STATUS_LED_PORT, SYSTEM_STATUS_LED_PIN);
	gpio_clear(SYSTEM_ERROR_LED_PORT, SYSTEM_ERROR_LED_PIN);

	ee_printf("Address: %02X\r\n", dev_addr);
	ee_printf("Sending boot init command...\r\n");
	send_boot_init(&dev_addr);

	ee_printf("\r\n");
	ee_printf("Autoboot in %d seconds...\r\n",
		  BOOT_BACKDOOR_ENTRY_TIMEOUT_MS / 1000);

	while (1) {
		// run boot task
		BootTask();
	}

	return 0;
}
