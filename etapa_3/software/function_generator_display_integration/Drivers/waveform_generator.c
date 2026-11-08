#include "waveform_generator.h"

#include <stddef.h>
#include <stdint.h>

#include "stm32f4xx.h"
#include "system_clock.h"

// configuracao da saida

#define SINE_MASTER_SAMPLE_COUNT 250U
#define SINE_MAX_SAMPLE_RATE_HZ  200000U
#define SINE_OFFSET_MV           1600U
#define DAC_REFERENCE_MV         3300U
#define DAC_MAX_VALUE            4095U
#define SINE_SCALE               10000
#define APB1_TIMER_CLOCK_HZ      (SYSTEM_CLOCK_HZ / 2U)
#define TIMER_MAX_DIVISOR        65536U
#define SINE_TIMER_IRQ_PRIORITY  4U
#define DAC_CHANNEL1_CONFIG_MASK (DAC_CR_EN1 | DAC_CR_BOFF1 | DAC_CR_TEN1 | \
                                  DAC_CR_TSEL1 | DAC_CR_WAVE1 | DAC_CR_MAMP1 | \
                                  DAC_CR_DMAEN1 | DAC_CR_DMAUDRIE1)

// quantidades de pontos da senoide

static const uint16_t supported_sample_counts[] = {
    250U, 125U, 50U, 25U, 10U
};

// tabela mestre da senoide

// cosseno normalizado com 250 pontos, escalado de -10000 a 10000
static const int16_t sine_master_shape[SINE_MASTER_SAMPLE_COUNT] = {
	10000, 9997, 9987, 9972, 9950, 9921, 9887, 9846, 9799, 9745,
	9686, 9620, 9549, 9471, 9387, 9298, 9202, 9101, 8994, 8881,
	8763, 8639, 8510, 8375, 8235, 8090, 7940, 7785, 7624, 7459,
	7290, 7115, 6937, 6753, 6566, 6374, 6179, 5979, 5776, 5569,
	5358, 5144, 4927, 4707, 4484, 4258, 4029, 3798, 3564, 3328,
	3090, 2850, 2608, 2365, 2120, 1874, 1626, 1378, 1129, 879,
	628, 377, 126, -126, -377, -628, -879, -1129, -1378, -1626,
	-1874, -2120, -2365, -2608, -2850, -3090, -3328, -3564, -3798, -4029,
	-4258, -4484, -4707, -4927, -5144, -5358, -5569, -5776, -5979, -6179,
	-6374, -6566, -6753, -6937, -7115, -7290, -7459, -7624, -7785, -7940,
	-8090, -8235, -8375, -8510, -8639, -8763, -8881, -8994, -9101, -9202,
	-9298, -9387, -9471, -9549, -9620, -9686, -9745, -9799, -9846, -9887,
	-9921, -9950, -9972, -9987, -9997, -10000, -9997, -9987, -9972, -9950,
	-9921, -9887, -9846, -9799, -9745, -9686, -9620, -9549, -9471, -9387,
	-9298, -9202, -9101, -8994, -8881, -8763, -8639, -8510, -8375, -8235,
	-8090, -7940, -7785, -7624, -7459, -7290, -7115, -6937, -6753, -6566,
	-6374, -6179, -5979, -5776, -5569, -5358, -5144, -4927, -4707, -4484,
	-4258, -4029, -3798, -3564, -3328, -3090, -2850, -2608, -2365, -2120,
	-1874, -1626, -1378, -1129, -879, -628, -377, -126, 126, 377,
	628, 879, 1129, 1378, 1626, 1874, 2120, 2365, 2608, 2850,
	3090, 3328, 3564, 3798, 4029, 4258, 4484, 4707, 4927, 5144,
	5358, 5569, 5776, 5979, 6179, 6374, 6566, 6753, 6937, 7115,
	7290, 7459, 7624, 7785, 7940, 8090, 8235, 8375, 8510, 8639,
	8763, 8881, 8994, 9101, 9202, 9298, 9387, 9471, 9549, 9620,
	9686, 9745, 9799, 9846, 9887, 9921, 9950, 9972, 9987, 9997
};

static uint16_t sine_samples[SINE_MASTER_SAMPLE_COUNT];

static volatile uint32_t sample_index;
static volatile uint32_t active_sample_count;
static volatile bool square_output_high;
static uint32_t square_high_ticks;
static uint32_t square_low_ticks;
static uint16_t square_high_code;
static uint16_t square_low_code;
static waveform_type_t current_type;
static uint32_t current_frequency_hz;
static uint32_t current_amplitude_mvpp;
static uint32_t current_duty_percent;
static bool generator_initialized;

static bool waveform_type_is_valid(waveform_type_t type)
{
	return (type == WAVEFORM_TYPE_SINE) || (type == WAVEFORM_TYPE_SQUARE);
}

// conversao entre tensao e codigo do dac

static uint16_t millivolts_to_dac_code(uint32_t voltage_mv)
{
	return (uint16_t)((((uint64_t)voltage_mv * DAC_MAX_VALUE) +
						(DAC_REFERENCE_MV / 2U)) /
					   DAC_REFERENCE_MV);
}

static uint16_t offset_dac_code(void)
{
	return millivolts_to_dac_code(SINE_OFFSET_MV);
}

// escolha automatica da quantidade de pontos

static uint32_t select_sample_count(uint32_t frequency_hz)
{
	uint32_t index;

	if ((frequency_hz < WAVEFORM_GENERATOR_MIN_FREQUENCY_HZ) ||
		(frequency_hz > WAVEFORM_GENERATOR_MAX_FREQUENCY_HZ))
	{
		return 0U;
	}

	for (index = 0U;
		 index < (sizeof(supported_sample_counts) / sizeof(supported_sample_counts[0]));
		 index++)
	{
		if (((uint64_t)frequency_hz * supported_sample_counts[index]) <=
			SINE_MAX_SAMPLE_RATE_HZ)
		{
			return supported_sample_counts[index];
		}
	}

	return 0U;
}

// calculo dos niveis de saida

static void update_sine_samples(uint32_t amplitude_mvpp, uint32_t sample_count)
{
	uint32_t index;
	uint32_t master_step;
	int64_t voltage_mv;

	master_step = SINE_MASTER_SAMPLE_COUNT / sample_count;

	for (index = 0U; index < sample_count; index++)
	{
		voltage_mv = (int64_t)SINE_OFFSET_MV +
					 (((int64_t)amplitude_mvpp *
					   sine_master_shape[index * master_step]) /
					  (2 * SINE_SCALE));
		sine_samples[index] = millivolts_to_dac_code((uint32_t)voltage_mv);
	}
}

static void update_square_levels(uint32_t amplitude_mvpp)
{
	const uint32_t half_amplitude_mv = amplitude_mvpp / 2U;

	square_low_code = millivolts_to_dac_code(SINE_OFFSET_MV - half_amplitude_mv);
	square_high_code = millivolts_to_dac_code(
		SINE_OFFSET_MV + amplitude_mvpp - half_amplitude_mv);
}

// calculo do timer

static bool calculate_sine_timer(uint32_t frequency_hz,
								 uint32_t sample_count,
								 uint32_t *prescaler,
								 uint32_t *period)
{
	uint64_t sample_rate_hz;
	uint64_t prescaler_divisor;
	uint64_t period_divisor;
	uint64_t period_ticks;

	if ((sample_count == 0U) || (prescaler == NULL) || (period == NULL))
	{
		return false;
	}

	sample_rate_hz = (uint64_t)frequency_hz * sample_count;
	prescaler_divisor = ((uint64_t)APB1_TIMER_CLOCK_HZ +
						 (sample_rate_hz * TIMER_MAX_DIVISOR) - 1U) /
						(sample_rate_hz * TIMER_MAX_DIVISOR);

	if ((prescaler_divisor == 0U) ||
		(prescaler_divisor > TIMER_MAX_DIVISOR))
	{
		return false;
	}

	period_divisor = prescaler_divisor * sample_rate_hz;
	period_ticks = ((uint64_t)APB1_TIMER_CLOCK_HZ +
					(period_divisor / 2U)) / period_divisor;

	if ((period_ticks == 0U) || (period_ticks > TIMER_MAX_DIVISOR))
	{
		return false;
	}

	*prescaler = (uint32_t)(prescaler_divisor - 1U);
	*period = (uint32_t)(period_ticks - 1U);
	return true;
}

static bool calculate_square_timer(uint32_t frequency_hz,
								   uint32_t duty_percent,
								   uint32_t *prescaler,
								   uint32_t *high_ticks,
								   uint32_t *low_ticks)
{
	uint64_t prescaler_divisor;
	uint64_t period_divisor;
	uint64_t period_ticks;
	uint64_t calculated_high_ticks;

	if ((frequency_hz < WAVEFORM_GENERATOR_MIN_FREQUENCY_HZ) ||
		(frequency_hz > WAVEFORM_GENERATOR_MAX_FREQUENCY_HZ) ||
		(duty_percent > WAVEFORM_GENERATOR_MAX_DUTY_PERCENT) ||
		(prescaler == NULL) || (high_ticks == NULL) || (low_ticks == NULL))
	{
		return false;
	}

	prescaler_divisor = ((uint64_t)APB1_TIMER_CLOCK_HZ +
						 ((uint64_t)frequency_hz * TIMER_MAX_DIVISOR) - 1U) /
						((uint64_t)frequency_hz * TIMER_MAX_DIVISOR);
	if ((prescaler_divisor == 0U) ||
		(prescaler_divisor > TIMER_MAX_DIVISOR))
	{
		return false;
	}

	period_divisor = prescaler_divisor * frequency_hz;
	period_ticks = ((uint64_t)APB1_TIMER_CLOCK_HZ +
					(period_divisor / 2U)) / period_divisor;
	if ((period_ticks < 2U) || (period_ticks > TIMER_MAX_DIVISOR))
	{
		return false;
	}

	calculated_high_ticks = ((period_ticks * duty_percent) + 50U) / 100U;
	if ((duty_percent > 0U) && (calculated_high_ticks == 0U))
	{
		calculated_high_ticks = 1U;
	}
	if ((duty_percent < 100U) && (calculated_high_ticks >= period_ticks))
	{
		calculated_high_ticks = period_ticks - 1U;
	}

	*prescaler = (uint32_t)(prescaler_divisor - 1U);
	*high_ticks = (uint32_t)calculated_high_ticks;
	*low_ticks = (uint32_t)(period_ticks - calculated_high_ticks);
	return true;
}

static void configure_timer(uint32_t prescaler, uint32_t period_ticks)
{
	TIM6->PSC = prescaler;
	TIM6->ARR = period_ticks - 1U;
	TIM6->CNT = 0U;
	TIM6->EGR = TIM_EGR_UG;
	TIM6->SR = 0U;
}

// aplicacao da configuracao atual

static bool apply_current_configuration(void)
{
	uint32_t sample_count;
	uint32_t prescaler;
	uint32_t period;
	bool timer_required = true;

	NVIC_DisableIRQ(TIM6_DAC_IRQn);
	CLEAR_BIT(TIM6->CR1, TIM_CR1_CEN);
	CLEAR_BIT(TIM6->DIER, TIM_DIER_UIE);

	if (current_type == WAVEFORM_TYPE_SINE)
	{
		sample_count = select_sample_count(current_frequency_hz);
		if (!calculate_sine_timer(current_frequency_hz,
								  sample_count,
								  &prescaler,
								  &period))
		{
			return false;
		}

		update_sine_samples(current_amplitude_mvpp, sample_count);
		active_sample_count = sample_count;
		sample_index = 1U;
		DAC1->DHR12R1 = sine_samples[0];
		TIM6->CR1 = TIM_CR1_ARPE;
		configure_timer(prescaler, period + 1U);
	}
	else
	{
		if (!calculate_square_timer(current_frequency_hz,
									current_duty_percent,
									&prescaler,
									&square_high_ticks,
									&square_low_ticks))
		{
			return false;
		}

		active_sample_count = 0U;
		update_square_levels(current_amplitude_mvpp);
		TIM6->CR1 = 0U;

		if ((current_duty_percent == 0U) || (current_amplitude_mvpp == 0U))
		{
			DAC1->DHR12R1 = square_low_code;
			timer_required = false;
		}
		else if (current_duty_percent == 100U)
		{
			DAC1->DHR12R1 = square_high_code;
			timer_required = false;
		}
		else
		{
			square_output_high = true;
			DAC1->DHR12R1 = square_high_code;
			configure_timer(prescaler, square_high_ticks);
		}
	}

	NVIC_ClearPendingIRQ(TIM6_DAC_IRQn);
	if (timer_required)
	{
		SET_BIT(TIM6->DIER, TIM_DIER_UIE);
		NVIC_EnableIRQ(TIM6_DAC_IRQn);
		SET_BIT(TIM6->CR1, TIM_CR1_CEN);
	}

	return true;
}

static bool settings_are_valid(waveform_type_t type,
							   uint32_t frequency_hz,
							   uint32_t amplitude_mvpp,
							   uint32_t duty_percent)
{
	return waveform_type_is_valid(type) &&
		   (frequency_hz >= WAVEFORM_GENERATOR_MIN_FREQUENCY_HZ) &&
		   (frequency_hz <= WAVEFORM_GENERATOR_MAX_FREQUENCY_HZ) &&
		   (amplitude_mvpp <= WAVEFORM_GENERATOR_MAX_AMPLITUDE_MVPP) &&
		   (duty_percent <= WAVEFORM_GENERATOR_MAX_DUTY_PERCENT);
}

// controle do gerador

bool waveform_generator_start(waveform_type_t type,
							  uint32_t frequency_hz,
							  uint32_t amplitude_mvpp,
							  uint32_t duty_percent)
{
	if (generator_initialized ||
		!settings_are_valid(type, frequency_hz, amplitude_mvpp, duty_percent))
	{
		return false;
	}

	NVIC_DisableIRQ(TIM6_DAC_IRQn);
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_DACEN | RCC_APB1ENR_TIM6EN);
	(void)READ_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM6EN);

	CLEAR_BIT(TIM6->CR1, TIM_CR1_CEN);
	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER4, GPIO_MODER_MODER4);
	CLEAR_BIT(GPIOA->PUPDR, GPIO_PUPDR_PUPDR4);

	CLEAR_BIT(DAC1->CR, DAC_CHANNEL1_CONFIG_MASK);
	DAC1->DHR12R1 = offset_dac_code();
	SET_BIT(DAC1->CR, DAC_CR_EN1);

	current_type = type;
	current_frequency_hz = frequency_hz;
	current_amplitude_mvpp = amplitude_mvpp;
	current_duty_percent = duty_percent;
	generator_initialized = true;

	NVIC_ClearPendingIRQ(TIM6_DAC_IRQn);
	NVIC_SetPriority(TIM6_DAC_IRQn, SINE_TIMER_IRQ_PRIORITY);
	if (!apply_current_configuration())
	{
		waveform_generator_stop();
		return false;
	}

	return true;
}

void waveform_generator_stop(void)
{
	if (!generator_initialized)
	{
		return;
	}

	CLEAR_BIT(TIM6->CR1, TIM_CR1_CEN);
	CLEAR_BIT(TIM6->DIER, TIM_DIER_UIE);
	NVIC_DisableIRQ(TIM6_DAC_IRQn);
	DAC1->DHR12R1 = offset_dac_code();
	active_sample_count = 0U;
	generator_initialized = false;
}

bool waveform_generator_set_type(waveform_type_t type)
{
	waveform_type_t previous_type;

	if (!generator_initialized || !waveform_type_is_valid(type))
	{
		return false;
	}
	if (type == current_type)
	{
		return true;
	}

	previous_type = current_type;
	CLEAR_BIT(TIM6->CR1, TIM_CR1_CEN);
	NVIC_DisableIRQ(TIM6_DAC_IRQn);
	DAC1->DHR12R1 = offset_dac_code();
	current_type = type;
	if (!apply_current_configuration())
	{
		current_type = previous_type;
		(void)apply_current_configuration();
		return false;
	}
	return true;
}

bool waveform_generator_set_frequency(uint32_t frequency_hz)
{
	uint32_t previous_frequency_hz;

	if (!generator_initialized ||
		(frequency_hz < WAVEFORM_GENERATOR_MIN_FREQUENCY_HZ) ||
		(frequency_hz > WAVEFORM_GENERATOR_MAX_FREQUENCY_HZ))
	{
		return false;
	}

	previous_frequency_hz = current_frequency_hz;
	current_frequency_hz = frequency_hz;
	if (!apply_current_configuration())
	{
		current_frequency_hz = previous_frequency_hz;
		(void)apply_current_configuration();
		return false;
	}
	return true;
}

bool waveform_generator_set_amplitude(uint32_t amplitude_mvpp)
{
	uint32_t previous_amplitude_mvpp;

	if (!generator_initialized ||
		(amplitude_mvpp > WAVEFORM_GENERATOR_MAX_AMPLITUDE_MVPP))
	{
		return false;
	}

	previous_amplitude_mvpp = current_amplitude_mvpp;
	current_amplitude_mvpp = amplitude_mvpp;
	if (!apply_current_configuration())
	{
		current_amplitude_mvpp = previous_amplitude_mvpp;
		(void)apply_current_configuration();
		return false;
	}
	return true;
}

bool waveform_generator_set_duty(uint32_t duty_percent)
{
	uint32_t previous_duty_percent;

	if (!generator_initialized ||
		(duty_percent > WAVEFORM_GENERATOR_MAX_DUTY_PERCENT))
	{
		return false;
	}

	previous_duty_percent = current_duty_percent;
	current_duty_percent = duty_percent;
	if ((current_type == WAVEFORM_TYPE_SQUARE) && !apply_current_configuration())
	{
		current_duty_percent = previous_duty_percent;
		(void)apply_current_configuration();
		return false;
	}
	return true;
}

waveform_type_t waveform_generator_get_type(void)
{
	return current_type;
}

uint32_t waveform_generator_get_frequency(void)
{
	return current_frequency_hz;
}

uint32_t waveform_generator_get_amplitude(void)
{
	return current_amplitude_mvpp;
}

uint32_t waveform_generator_get_duty(void)
{
	return current_duty_percent;
}

uint32_t waveform_generator_get_sample_count(void)
{
	return active_sample_count;
}

// atualizacao do dac por interrupcao

void TIM6_DAC_IRQHandler(void)
{
	if (READ_BIT(TIM6->SR, TIM_SR_UIF) == 0U)
	{
		return;
	}

	TIM6->SR = 0U;
	if (current_type == WAVEFORM_TYPE_SINE)
	{
		DAC1->DHR12R1 = sine_samples[sample_index];
		sample_index++;
		if (sample_index >= active_sample_count)
		{
			sample_index = 0U;
		}
	}
	else if (square_output_high)
	{
		DAC1->DHR12R1 = square_low_code;
		square_output_high = false;
		TIM6->ARR = square_low_ticks - 1U;
	}
	else
	{
		DAC1->DHR12R1 = square_high_code;
		square_output_high = true;
		TIM6->ARR = square_high_ticks - 1U;
	}
}

