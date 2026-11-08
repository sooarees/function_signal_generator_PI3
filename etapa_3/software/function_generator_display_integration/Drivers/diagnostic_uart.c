#include "diagnostic_uart.h"

#include <stddef.h>

#include "board.h"

// configuracao da serial

#define DIAGNOSTIC_UART_BAUD_RATE 115200U
#define DIAGNOSTIC_UART_IRQ_PRIORITY 7U
#define DIAGNOSTIC_UART_RX_BUFFER_SIZE 128U

static volatile uint8_t rx_buffer[DIAGNOSTIC_UART_RX_BUFFER_SIZE];
static volatile uint32_t rx_head;
static volatile uint32_t rx_tail;

// clock e transmissao

static uint32_t diagnostic_uart_pclk1(void)
{
	const uint32_t prescaler_code = (RCC->CFGR >> 10U) & 0x7U;
	uint32_t peripheral_clock = SystemCoreClock;

	if (prescaler_code >= 4U)
	{
		peripheral_clock >>= prescaler_code - 3U;
	}

	return peripheral_clock;
}

static void diagnostic_uart_write_character(char character)
{
	while ((BOARD_DIAGNOSTIC_UART->SR & USART_SR_TXE) == 0U)
	{
	}

	BOARD_DIAGNOSTIC_UART->DR = (uint32_t)(uint8_t)character;
}

// inicializacao da serial

void diagnostic_uart_init(void)
{
	const uint32_t mode_shift = BOARD_DIAGNOSTIC_UART_TX_PIN_NUMBER * 2U;
	const uint32_t mode_mask = 3UL << mode_shift;
	const uint32_t alternate_shift =
		(BOARD_DIAGNOSTIC_UART_TX_PIN_NUMBER - 8U) * 4U;
	const uint32_t alternate_mask = 0xFUL << alternate_shift;
	const uint32_t tx_pin = 1UL << BOARD_DIAGNOSTIC_UART_TX_PIN_NUMBER;
	const uint32_t rx_pin = 1UL << BOARD_DIAGNOSTIC_UART_RX_PIN_NUMBER;
	const uint32_t peripheral_clock = diagnostic_uart_pclk1();

	SET_BIT(RCC->AHB1ENR, BOARD_DIAGNOSTIC_UART_TX_GPIO_CLOCK);
	SET_BIT(RCC->APB1ENR, BOARD_DIAGNOSTIC_UART_CLOCK_ENABLE);
	(void)READ_BIT(RCC->APB1ENR, BOARD_DIAGNOSTIC_UART_CLOCK_ENABLE);

	BOARD_DIAGNOSTIC_UART->CR1 = 0U;
	BOARD_DIAGNOSTIC_UART->CR2 = 0U;
	BOARD_DIAGNOSTIC_UART->CR3 = 0U;
	BOARD_DIAGNOSTIC_UART->BRR =
		(peripheral_clock + (DIAGNOSTIC_UART_BAUD_RATE / 2U)) /
		DIAGNOSTIC_UART_BAUD_RATE;
	rx_head = 0U;
	rx_tail = 0U;

	BOARD_DIAGNOSTIC_UART_TX_GPIO->BSRR = tx_pin;
	CLEAR_BIT(BOARD_DIAGNOSTIC_UART_TX_GPIO->OTYPER, tx_pin);
	MODIFY_REG(BOARD_DIAGNOSTIC_UART_TX_GPIO->OSPEEDR,
			   mode_mask, 1UL << mode_shift);
	MODIFY_REG(BOARD_DIAGNOSTIC_UART_TX_GPIO->PUPDR,
			   mode_mask, 1UL << mode_shift);
	MODIFY_REG(BOARD_DIAGNOSTIC_UART_TX_GPIO->AFR[1], alternate_mask,
			   BOARD_DIAGNOSTIC_UART_TX_AF << alternate_shift);
	MODIFY_REG(BOARD_DIAGNOSTIC_UART_TX_GPIO->MODER,
			   mode_mask, 2UL << mode_shift);

	CLEAR_BIT(BOARD_DIAGNOSTIC_UART_TX_GPIO->OTYPER, rx_pin);
	MODIFY_REG(BOARD_DIAGNOSTIC_UART_TX_GPIO->PUPDR,
			   GPIO_PUPDR_PUPD9, GPIO_PUPDR_PUPD9_0);
	MODIFY_REG(BOARD_DIAGNOSTIC_UART_TX_GPIO->AFR[1], GPIO_AFRH_AFSEL9,
			   BOARD_DIAGNOSTIC_UART_RX_AF << GPIO_AFRH_AFSEL9_Pos);
	MODIFY_REG(BOARD_DIAGNOSTIC_UART_TX_GPIO->MODER,
			   GPIO_MODER_MODE9, GPIO_MODER_MODE9_1);

	NVIC_ClearPendingIRQ(USART3_IRQn);
	NVIC_SetPriority(USART3_IRQn, DIAGNOSTIC_UART_IRQ_PRIORITY);
	NVIC_EnableIRQ(USART3_IRQn);
	BOARD_DIAGNOSTIC_UART->CR1 = USART_CR1_TE | USART_CR1_RE |
								 USART_CR1_RXNEIE | USART_CR1_UE;
}

// envio de dados

void diagnostic_uart_write(const char *text)
{
	while (*text != '\0')
	{
		diagnostic_uart_write_character(*text);
		text++;
	}
}

void diagnostic_uart_write_u32(uint32_t value)
{
	char digits[10];
	uint32_t digit_count = 0U;

	do
	{
		digits[digit_count] = (char)('0' + (value % 10U));
		digit_count++;
		value /= 10U;
	}
	while (value > 0U);

	while (digit_count > 0U)
	{
		digit_count--;
		diagnostic_uart_write_character(digits[digit_count]);
	}
}

// leitura dos dados recebidos

bool diagnostic_uart_try_read(char *character)
{
	if (character == NULL)
	{
		return false;
	}

	if (rx_tail == rx_head)
	{
		return false;
	}

	*character = (char)rx_buffer[rx_tail];
	rx_tail = (rx_tail + 1U) % DIAGNOSTIC_UART_RX_BUFFER_SIZE;
	return true;
}

// recepcao por interrupcao

void USART3_IRQHandler(void)
{
	uint32_t status;
	uint32_t received_data;
	uint32_t next_head;

	status = BOARD_DIAGNOSTIC_UART->SR;
	if ((status & (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE)) != 0U)
	{
		(void)BOARD_DIAGNOSTIC_UART->DR;
		return;
	}

	if ((status & USART_SR_RXNE) != 0U)
	{
		received_data = BOARD_DIAGNOSTIC_UART->DR & 0xFFU;
		next_head = (rx_head + 1U) % DIAGNOSTIC_UART_RX_BUFFER_SIZE;

		if (next_head != rx_tail)
		{
			rx_buffer[rx_head] = (uint8_t)received_data;
			rx_head = next_head;
		}
	}
}
