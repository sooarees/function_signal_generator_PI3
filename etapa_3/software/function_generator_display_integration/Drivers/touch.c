#include "touch.h"

#include "board.h"
#include "display.h"

// limites e calibracao

#define TOUCH_ADC_MAX              4095U
#define TOUCH_RX_PLATE_OHMS        300U
#define TOUCH_MIN_PRESSURE_OHMS    180U
#define TOUCH_MAX_PRESSURE_OHMS    1000U
#define TOUCH_SAMPLE_NOISE         16U
#define TOUCH_ADC_TIMEOUT          100000U

#define TOUCH_FROM_10_BIT(value) \
	((((uint32_t)(value) * TOUCH_ADC_MAX) + 511U) / 1023U)

#define TOUCH_LEFT  TOUCH_FROM_10_BIT(144U)
#define TOUCH_RIGHT TOUCH_FROM_10_BIT(883U)
#define TOUCH_TOP   TOUCH_FROM_10_BIT(109U)
#define TOUCH_BOTTOM TOUCH_FROM_10_BIT(870U)

// configuracao dos pinos

static uint32_t gpio_pin_number(uint32_t pin)
{
	uint32_t number = 0U;

	while (((pin >> number) & 1U) == 0U)
	{
		number++;
	}
	return number;
}

static void gpio_mode(GPIO_TypeDef *gpio, uint32_t pin, uint32_t mode)
{
	const uint32_t shift = gpio_pin_number(pin) * 2U;
	const uint32_t mask = 3UL << shift;

	MODIFY_REG(gpio->MODER, mask, mode << shift);
	CLEAR_BIT(gpio->PUPDR, mask);
}

static void gpio_output(GPIO_TypeDef *gpio, uint32_t pin, bool high)
{
	gpio->BSRR = high ? pin : (pin << 16U);
	CLEAR_BIT(gpio->OTYPER, pin);
	gpio_mode(gpio, pin, 1U);
}

static void gpio_input(GPIO_TypeDef *gpio, uint32_t pin)
{
	gpio_mode(gpio, pin, 0U);
}

static void gpio_analog(GPIO_TypeDef *gpio, uint32_t pin)
{
	gpio_mode(gpio, pin, 3U);
}

static void touch_release_electrodes(void)
{
	// interrompe os caminhos de corrente antes de trocar o eixo medido.
	gpio_input(BOARD_TOUCH_XP_GPIO, BOARD_TOUCH_XP_PIN);
	gpio_analog(BOARD_TOUCH_XM_GPIO, BOARD_TOUCH_XM_PIN);
	gpio_analog(BOARD_TOUCH_YP_GPIO, BOARD_TOUCH_YP_PIN);
	gpio_input(BOARD_TOUCH_YM_GPIO, BOARD_TOUCH_YM_PIN);
	__DSB();
}

// leitura do adc

static void touch_delay_us(uint32_t microseconds)
{
	const uint32_t cycles = (SystemCoreClock / 1000000U) * microseconds;
	const uint32_t start = DWT->CYCCNT;

	while ((DWT->CYCCNT - start) < cycles)
	{
	}
}

static bool adc_read(uint8_t channel, uint16_t *value)
{
	uint32_t timeout = TOUCH_ADC_TIMEOUT;

	(void)BOARD_TOUCH_ADC->DR;
	MODIFY_REG(BOARD_TOUCH_ADC->SQR3, ADC_SQR3_SQ1,
			   (uint32_t)channel << ADC_SQR3_SQ1_Pos);
	SET_BIT(BOARD_TOUCH_ADC->CR2, ADC_CR2_SWSTART);

	while (((BOARD_TOUCH_ADC->SR & ADC_SR_EOC) == 0U) && (timeout > 0U))
	{
		timeout--;
	}
	if (timeout == 0U)
	{
		return false;
	}

	*value = (uint16_t)BOARD_TOUCH_ADC->DR;
	return true;
}

static bool adc_read_stable(uint8_t channel, uint16_t *value)
{
	uint16_t first;
	uint16_t second;
	uint16_t difference;

	if (!adc_read(channel, &first) || !adc_read(channel, &second))
	{
		return false;
	}
	difference = first > second ? (uint16_t)(first - second)
		: (uint16_t)(second - first);
	if (difference > TOUCH_SAMPLE_NOISE)
	{
		return false;
	}

	*value = (uint16_t)(((uint32_t)first + second) / 2U);
	return true;
}

static int16_t touch_map(uint16_t value, uint16_t input_start,
						 uint16_t input_end, int16_t output_end)
{
	const int32_t numerator = ((int32_t)value - input_start) * output_end;
	const int32_t denominator = (int32_t)input_end - input_start;
	int32_t mapped = numerator / denominator;

	if (mapped < 0)
	{
		mapped = 0;
	}
	if (mapped >= output_end)
	{
		mapped = output_end - 1;
	}
	return (int16_t)mapped;
}

// inicializacao do touch

void touch_init(void)
{
	SET_BIT(RCC->APB2ENR, BOARD_TOUCH_ADC_CLOCK);
	(void)READ_BIT(RCC->APB2ENR, BOARD_TOUCH_ADC_CLOCK);

	MODIFY_REG(ADC123_COMMON->CCR, ADC_CCR_ADCPRE, ADC_CCR_ADCPRE_0);
	BOARD_TOUCH_ADC->CR1 = 0U;
	BOARD_TOUCH_ADC->CR2 = 0U;
	MODIFY_REG(BOARD_TOUCH_ADC->SMPR1,
			   ADC_SMPR1_SMP10 | ADC_SMPR1_SMP13,
			   ADC_SMPR1_SMP10_2 | ADC_SMPR1_SMP13_2);
	BOARD_TOUCH_ADC->SQR1 = 0U;
	SET_BIT(BOARD_TOUCH_ADC->CR2, ADC_CR2_ADON);
	touch_delay_us(10U);
	display_restore_after_touch();
}

// leitura das coordenadas e da pressao

bool touch_read(touch_point_t *point)
{
	uint16_t sample;
	uint16_t raw_x = 0U;
	uint16_t raw_y = 0U;
	uint16_t z1 = 0U;
	uint16_t z2 = 0U;
	uint32_t pressure = 0U;
	bool valid = true;

	display_prepare_touch();

	// eixo x: alimenta os eletrodos x e mede y+ no canal 10 do adc.
	touch_release_electrodes();
	gpio_output(BOARD_TOUCH_XP_GPIO, BOARD_TOUCH_XP_PIN, true);
	gpio_output(BOARD_TOUCH_XM_GPIO, BOARD_TOUCH_XM_PIN, false);
	touch_delay_us(20U);
	valid = adc_read_stable(BOARD_TOUCH_YP_ADC_CHANNEL, &sample);
	if (valid)
	{
		raw_x = (uint16_t)(TOUCH_ADC_MAX - sample);
	}

	// eixo y: alimenta os eletrodos y e mede x- no canal 13 do adc.
	touch_release_electrodes();
	gpio_output(BOARD_TOUCH_YP_GPIO, BOARD_TOUCH_YP_PIN, true);
	gpio_output(BOARD_TOUCH_YM_GPIO, BOARD_TOUCH_YM_PIN, false);
	touch_delay_us(20U);
	if (!adc_read_stable(BOARD_TOUCH_XM_ADC_CHANNEL, &sample))
	{
		valid = false;
	}
	else
	{
		raw_y = (uint16_t)(TOUCH_ADC_MAX - sample);
	}

	// pressao: x+ baixo, y- alto e os outros dois pinos em alta impedancia.
	touch_release_electrodes();
	gpio_output(BOARD_TOUCH_XP_GPIO, BOARD_TOUCH_XP_PIN, false);
	gpio_output(BOARD_TOUCH_YM_GPIO, BOARD_TOUCH_YM_PIN, true);
	touch_delay_us(20U);
	if (!adc_read(BOARD_TOUCH_XM_ADC_CHANNEL, &z1) ||
		!adc_read(BOARD_TOUCH_YP_ADC_CHANNEL, &z2))
	{
		valid = false;
	}

	if (valid && (z1 > 0U) && (z2 > z1))
	{
		const uint32_t ratio = ((uint32_t)(z2 - z1) * raw_x) / z1;

		if (ratio > (UINT32_MAX / TOUCH_RX_PLATE_OHMS))
		{
			pressure = UINT32_MAX;
		}
		else
		{
			pressure = (ratio * TOUCH_RX_PLATE_OHMS) /
				(TOUCH_ADC_MAX + 1U);
		}
	}
	else
	{
		valid = false;
	}

	display_restore_after_touch();

	point->raw_x = raw_x;
	point->raw_y = raw_y;
	point->pressure = pressure > UINT16_MAX ? UINT16_MAX : (uint16_t)pressure;
	// acompanha a rotacao 3 usada no display.
	point->x = touch_map(raw_y, TOUCH_BOTTOM, TOUCH_TOP, DISPLAY_WIDTH);
	point->y = touch_map(raw_x, TOUCH_LEFT, TOUCH_RIGHT, DISPLAY_HEIGHT);

	return valid && (pressure >= TOUCH_MIN_PRESSURE_OHMS) &&
		(pressure <= TOUCH_MAX_PRESSURE_OHMS);
}
