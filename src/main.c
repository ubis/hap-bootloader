/*
 *		      HAP-Bootoader, based on OpenBLT
 *
 *	@author		Jokubas Maciulaitis (ubis)
 *	@file		main.c
 *	@date		2019 04 19
 */

#include <boot.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

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
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

	// Error LED
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO5);

	// Turn on both LEDs
	gpio_set(GPIOC, GPIO13);
	gpio_set(GPIOB, GPIO5);

	// Setup USART
	usart_set_baudrate(USART1, BOOT_CONSOLE_UART_BAUDRATE);
	usart_set_databits(USART1, BOOT_CONSOLE_UART_DATA_BITS);
	usart_set_stopbits(USART1, BOOT_CONSOLE_UART_STOP_BITS);
	usart_set_mode(USART1, USART_MODE_TX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

	usart_enable(USART1);
}

blt_bool CpuUserProgramStartHook(void)
{
	// Turn off both LEDs
	gpio_clear(GPIOC, GPIO13);
	gpio_clear(GPIOB, GPIO5);

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
		gpio_set(GPIOC, GPIO13);
		gpio_clear(GPIOB, GPIO5);
	} else {
		ledOn = BLT_FALSE;
		gpio_clear(GPIOC, GPIO13);
		gpio_set(GPIOB, GPIO5);
	}

	nextBlinkEvent = TimerGet() + ledBlinkIntervalMs;
}

int main(void)
{
	// configure clock and peripherals
	cpu_init();

	// initialize the bootloader
	BootInit();

	// Turn off both LEDs
	gpio_clear(GPIOC, GPIO13);
	gpio_clear(GPIOB, GPIO5);

	while (1) {
		// run boot task
		BootTask();
	}

	return 0;
}
