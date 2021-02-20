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
#include <libopencm3/stm32/iwdg.h>

#include "protodef.h"

// unique stm32f1 id 96bit
#define U_ID_1 (*((unsigned int *) 0x1FFFF7E8))
#define U_ID_LEN 12

#define WATCHDOG_PERIOD_MS 5000

uint8_t __attribute__((section(".boot_version"))) g_boot_version[4] = {
	BOOT_VERSION_CORE_MAIN,
	BOOT_VERSION_CORE_MINOR,
	BOOT_VERSION_CORE_PATCH
};

uint8_t __attribute__((section(".boot_date"))) g_boot_date[8] = {
	BOOT_VERSION_CORE_DATE
};

static void cpu_init(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_USART1);
	rcc_periph_clock_enable(RCC_CAN1);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);

	// set all PA/PB/PC pins as input
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_ALL);
	gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_ALL);
	gpio_set_mode(GPIOC, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_ALL);

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

	// Setup USART
	usart_set_baudrate(USART1, BOOT_CONSOLE_UART_BAUDRATE);
	usart_set_databits(USART1, BOOT_CONSOLE_UART_DATA_BITS);
	usart_set_stopbits(USART1, BOOT_CONSOLE_UART_STOP_BITS);
	usart_set_mode(USART1, USART_MODE_TX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

	usart_enable(USART1);

	// Setup Watchdog
	rcc_osc_on(RCC_LSI);
	rcc_wait_for_osc_ready(RCC_LSI);

	iwdg_set_period_ms(WATCHDOG_PERIOD_MS);
	iwdg_start();
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

static void set_up_header(unsigned short *addr, unsigned char *ext)
{
	header_t g_header;

	// generate address from uid
	*addr = calc_crc16((const unsigned char *)&U_ID_1, U_ID_LEN);
	*ext = U_ID_1 & 0xFF;

	// set up can rx header
	g_header.mode = MODE_BOOT;
	g_header.reserved = 0;
	g_header.address = *addr;
	g_header.uid_part = *ext;
	g_header.identifier = ID_29_BIT;
	CanSetRxMsgId(g_header.id);

	// set can tx header
	CanSetTxMsgId(g_header.id);
}

static void send_boot_init(void)
{
	unsigned char buffer[2];

	buffer[0] = XCP_PID_NFY;
	buffer[1] = XCP_CMD_BOOT_INIT;

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
	iwdg_reset();
}

int main(void)
{
	unsigned short dev_addr;
	unsigned char ext_part;

	// configure clock and peripherals
	cpu_init();

	// set up CAN tx/rx header
	set_up_header(&dev_addr, &ext_part);

	// initialize the bootloader
	BootInit();

	ee_printf("\r\n");
	ee_printf("Address: %02X%04X\r\n", ext_part, dev_addr);
	ee_printf("Sending boot init command...\r\n");
	send_boot_init();

	ee_printf("\r\n");
	ee_printf("Autoboot in %d seconds...\r\n",
		  BOOT_BACKDOOR_ENTRY_TIMEOUT_MS / 1000);

	while (1) {
		// run boot task
		BootTask();
	}

	return 0;
}
