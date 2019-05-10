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

// unique stm32f1 id
#define U_ID_1 (*((unsigned int *) 0x1FFFF7E8))
#define U_ID_2 (*((unsigned int *) 0x1FFFF7EC))
#define U_ID_3 (*((unsigned int *) 0x1FFFF7F0))

static blt_int16u ledBlinkIntervalMs;

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
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, SYSTEM_ERROR_LED_PIN);

	// Turn on both LEDs
	gpio_set(SYSTEM_STATUS_LED_PORT, SYSTEM_STATUS_LED_PIN);
	gpio_set(GPIOB, SYSTEM_ERROR_LED_PIN);

	// Setup USART
	usart_set_baudrate(USART1, BOOT_CONSOLE_UART_BAUDRATE);
	usart_set_databits(USART1, BOOT_CONSOLE_UART_DATA_BITS);
	usart_set_stopbits(USART1, BOOT_CONSOLE_UART_STOP_BITS);
	usart_set_mode(USART1, USART_MODE_TX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

	usart_enable(USART1);
}

uint16_t calc_crc16(const uint8_t *addr, uint32_t len)
{
	const uint16_t mask = 0x8000;
	const uint16_t poly = 0x1021;
	uint16_t crc = 0xFFFF;
	uint8_t i;

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

blt_bool CpuUserProgramStartHook(void)
{
	// Turn off both LEDs
	gpio_clear(SYSTEM_STATUS_LED_PORT, SYSTEM_STATUS_LED_PIN);
	gpio_clear(SYSTEM_ERROR_LED_PORT, SYSTEM_ERROR_LED_PIN);

	return BLT_TRUE;
}

void CopInitHook(void)
{
	ledBlinkIntervalMs = 100;
}

void CopServiceHook(void)
{
	static blt_bool ledOn = BLT_FALSE;
	static blt_int32u nextBlinkEvent = 0;

	// check for blink event
	if (TimerGet() < nextBlinkEvent) {
		return;
	}

	// toggle the LED state
	if (ledOn == BLT_FALSE) {
		ledOn = BLT_TRUE;
		gpio_set(SYSTEM_STATUS_LED_PORT, SYSTEM_STATUS_LED_PIN);
		gpio_clear(SYSTEM_ERROR_LED_PORT, SYSTEM_ERROR_LED_PIN);
	} else {
		ledOn = BLT_FALSE;
		gpio_clear(SYSTEM_STATUS_LED_PORT, SYSTEM_STATUS_LED_PIN);
		gpio_set(SYSTEM_ERROR_LED_PORT, SYSTEM_ERROR_LED_PIN);
	}

	nextBlinkEvent = TimerGet() + ledBlinkIntervalMs;
}

int main(void)
{
	unsigned char id_arr[sizeof(unsigned int) * 3] = { 0 };
	const size_t id_len = sizeof(id_arr) / sizeof(id_arr[0]);
	uint16_t address;

	// configure clock and peripherals
	cpu_init();

	// generate address from uid
	memcpy(id_arr, &U_ID_1, 4);
	memcpy(id_arr + 4, &U_ID_2, 4);
	memcpy(id_arr + 8, &U_ID_3, 4);

	address = calc_crc16(&id_arr[0], id_len);
	memcpy(id_arr, &address, sizeof(address));

	// initialize the bootloader
	BootInit();

	ee_printf("\r\n");

	// turn off both LEDs
	gpio_clear(SYSTEM_STATUS_LED_PORT, SYSTEM_STATUS_LED_PIN);
	gpio_clear(SYSTEM_ERROR_LED_PORT, SYSTEM_ERROR_LED_PIN);

	ee_printf("Device address: 0x%02X\r\n", address);
	ee_printf("Sending boot init command...\r\n");
	CanTransmitPacket(&id_arr[0], sizeof(address));

	ee_printf("\r\n");

	while (1) {
		// run boot task
		BootTask();
	}

	return 0;
}
