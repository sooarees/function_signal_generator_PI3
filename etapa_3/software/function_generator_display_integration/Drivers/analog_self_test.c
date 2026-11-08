#include "analog_self_test.h"

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board.h"
#include "diagnostic_uart.h"
#include "system_clock.h"
#include "waveform_generator.h"

// limites do autoteste

#define SELF_TEST_ADC_MAX_CODE       4095U
#define SELF_TEST_REFERENCE_MV       3300U
#define SELF_TEST_MIN_CAPTURE_MS     100U
#define SELF_TEST_MAX_CAPTURE_MS     4000U
#define SELF_TEST_TARGET_CYCLES      4U
#define SELF_TEST_ADC_TIMEOUT        100000U
#define SELF_TEST_MIN_DYNAMIC_SPAN   16U
#define SELF_TEST_STATIC_SPAN_LIMIT  30U
#define SELF_TEST_MIN_CROSSINGS      3U

typedef struct
{
	uint16_t minimum;
	uint16_t maximum;
	uint32_t average;
	uint32_t sample_count;
} capture_result_t;

static bool self_test_running;

// exclusao entre chamadas do autoteste

static bool self_test_try_lock(void)
{
	bool acquired = false;

	taskENTER_CRITICAL();
	if (!self_test_running)
	{
		self_test_running = true;
		acquired = true;
	}
	taskEXIT_CRITICAL();
	return acquired;
}

static void self_test_unlock(void)
{
	taskENTER_CRITICAL();
	self_test_running = false;
	taskEXIT_CRITICAL();
}

// tempo de captura

static uint32_t select_capture_time_ms(uint32_t frequency_hz)
{
	uint32_t capture_ms;

	if (frequency_hz == 0U)
	{
		return SELF_TEST_MIN_CAPTURE_MS;
	}

	capture_ms = ((SELF_TEST_TARGET_CYCLES * 1000U) +
				  frequency_hz - 1U) / frequency_hz;
	if (capture_ms < SELF_TEST_MIN_CAPTURE_MS)
	{
		capture_ms = SELF_TEST_MIN_CAPTURE_MS;
	}
	if (capture_ms > SELF_TEST_MAX_CAPTURE_MS)
	{
		capture_ms = SELF_TEST_MAX_CAPTURE_MS;
	}
	return capture_ms;
}

// temporizacao curta

static void enable_cycle_counter(void)
{
	SET_BIT(CoreDebug->DEMCR, CoreDebug_DEMCR_TRCENA_Msk);
	SET_BIT(DWT->CTRL, DWT_CTRL_CYCCNTENA_Msk);
}

static void delay_us(uint32_t microseconds)
{
	const uint32_t delay_cycles =
		(SystemCoreClock / 1000000U) * microseconds;
	const uint32_t start = DWT->CYCCNT;

	while ((DWT->CYCCNT - start) < delay_cycles)
	{
	}
}

// leitura do canal de monitoramento

static bool adc2_read(uint16_t *value)
{
	uint32_t timeout = SELF_TEST_ADC_TIMEOUT;

	(void)BOARD_SIGNAL_MONITOR_ADC->DR;
	SET_BIT(BOARD_SIGNAL_MONITOR_ADC->CR2, ADC_CR2_SWSTART);
	while (((BOARD_SIGNAL_MONITOR_ADC->SR & ADC_SR_EOC) == 0U) &&
		   (timeout > 0U))
	{
		timeout--;
	}
	if (timeout == 0U)
	{
		return false;
	}

	*value = (uint16_t)BOARD_SIGNAL_MONITOR_ADC->DR;
	return true;
}

static void monitor_adc_init(void)
{
	enable_cycle_counter();
	SET_BIT(RCC->APB2ENR, BOARD_SIGNAL_MONITOR_ADC_CLOCK);
	(void)READ_BIT(RCC->APB2ENR, BOARD_SIGNAL_MONITOR_ADC_CLOCK);

	// adc1 do touch e adc2 do autoteste usam o mesmo prescaler seguro, /4.
	MODIFY_REG(ADC123_COMMON->CCR, ADC_CCR_ADCPRE, ADC_CCR_ADCPRE_0);
	BOARD_SIGNAL_MONITOR_ADC->CR1 = 0U;
	BOARD_SIGNAL_MONITOR_ADC->CR2 = 0U;
	MODIFY_REG(BOARD_SIGNAL_MONITOR_ADC->SMPR2,
			   ADC_SMPR2_SMP4,
			   ADC_SMPR2_SMP4_2);
	BOARD_SIGNAL_MONITOR_ADC->SQR1 = 0U;
	BOARD_SIGNAL_MONITOR_ADC->SQR3 =
		BOARD_SIGNAL_MONITOR_ADC_CHANNEL << ADC_SQR3_SQ1_Pos;
	SET_BIT(BOARD_SIGNAL_MONITOR_ADC->CR2, ADC_CR2_ADON);
	delay_us(10U);
}

static void monitor_adc_stop(void)
{
	CLEAR_BIT(BOARD_SIGNAL_MONITOR_ADC->CR2, ADC_CR2_ADON);
	CLEAR_BIT(RCC->APB2ENR, BOARD_SIGNAL_MONITOR_ADC_CLOCK);
}

// captura da amplitude

static bool capture_levels(uint32_t capture_ms, capture_result_t *result)
{
	const uint32_t capture_cycles =
		(SystemCoreClock / 1000U) * capture_ms;
	const uint32_t start = DWT->CYCCNT;
	uint64_t sum = 0U;
	uint32_t count = 0U;
	uint16_t minimum = UINT16_MAX;
	uint16_t maximum = 0U;
	uint16_t sample;

	while ((DWT->CYCCNT - start) < capture_cycles)
	{
		if (!adc2_read(&sample))
		{
			return false;
		}
		if (sample < minimum)
		{
			minimum = sample;
		}
		if (sample > maximum)
		{
			maximum = sample;
		}
		sum += sample;
		count++;
	}

	if (count == 0U)
	{
		return false;
	}

	result->minimum = minimum;
	result->maximum = maximum;
	result->average = (uint32_t)(sum / count);
	result->sample_count = count;
	return true;
}

// captura da frequencia

static bool capture_frequency(uint16_t minimum,
							  uint16_t maximum,
							  uint32_t capture_ms,
							  uint32_t *frequency_hz,
							  uint32_t *crossings)
{
	const uint32_t capture_cycles =
		(SystemCoreClock / 1000U) * capture_ms;
	const uint32_t span = (uint32_t)maximum - minimum;
	const uint16_t low_threshold =
		(uint16_t)(minimum + ((span * 3U) / 8U));
	const uint16_t high_threshold =
		(uint16_t)(minimum + ((span * 5U) / 8U));
	const uint32_t start = DWT->CYCCNT;
	uint32_t first_crossing = 0U;
	uint32_t last_crossing = 0U;
	uint32_t crossing_count = 0U;
	uint16_t sample;
	bool armed = false;

	while ((DWT->CYCCNT - start) < capture_cycles)
	{
		if (!adc2_read(&sample))
		{
			return false;
		}
		if (sample <= low_threshold)
		{
			armed = true;
		}
		else if (armed && (sample >= high_threshold))
		{
			const uint32_t now = DWT->CYCCNT;

			if (crossing_count == 0U)
			{
				first_crossing = now;
			}
			last_crossing = now;
			crossing_count++;
			armed = false;
		}
	}

	*crossings = crossing_count;
	if ((crossing_count < SELF_TEST_MIN_CROSSINGS) ||
		(last_crossing == first_crossing))
	{
		*frequency_hz = 0U;
		return true;
	}

	*frequency_hz = (uint32_t)((
		((uint64_t)(crossing_count - 1U) * SystemCoreClock) +
		((last_crossing - first_crossing) / 2U)) /
		(last_crossing - first_crossing));
	return true;
}

// conversao e validacao

static uint32_t adc_code_to_mv(uint32_t code)
{
	return ((code * SELF_TEST_REFERENCE_MV) +
			(SELF_TEST_ADC_MAX_CODE / 2U)) / SELF_TEST_ADC_MAX_CODE;
}

static bool value_within_tolerance(uint32_t measured,
								   uint32_t expected,
								   uint32_t tolerance)
{
	const uint32_t lower = expected > tolerance ? expected - tolerance : 0U;
	const uint32_t upper = expected + tolerance;

	return (measured >= lower) && (measured <= upper);
}

// saida dos resultados

static void write_measurement(const char *label, uint32_t value,
							  const char *unit)
{
	diagnostic_uart_write(label);
	diagnostic_uart_write_u32(value);
	diagnostic_uart_write(unit);
}

// execucao do autoteste

bool analog_self_test_run(void)
{
	capture_result_t capture;
	const uint32_t expected_frequency_hz =
		waveform_generator_get_frequency();
	const uint32_t expected_amplitude_mvpp =
		waveform_generator_get_amplitude();
	const waveform_type_t waveform = waveform_generator_get_type();
	const uint32_t duty_percent = waveform_generator_get_duty();
	uint32_t measured_frequency_hz = 0U;
	uint32_t crossing_count = 0U;
	uint32_t measured_amplitude_mvpp;
	uint32_t amplitude_tolerance_mv;
	uint32_t frequency_tolerance_hz;
	bool dynamic_signal;
	bool amplitude_ok;
	bool frequency_ok;
	bool result_ok;
	const uint32_t capture_ms =
		select_capture_time_ms(expected_frequency_hz);

	if (!self_test_try_lock())
	{
		diagnostic_uart_write("autoteste ocupado; tente novamente\r\n");
		return false;
	}

	diagnostic_uart_write("\r\nautoteste analogico PA4 via ADC2\r\n");
	write_measurement("janela_por_etapa=", capture_ms, " ms\r\n");
	monitor_adc_init();
	if (!capture_levels(capture_ms, &capture))
	{
		monitor_adc_stop();
		diagnostic_uart_write("resultado=FALHA | ADC2 sem conversao\r\n");
		self_test_unlock();
		return false;
	}

	measured_amplitude_mvpp = adc_code_to_mv(
		(uint32_t)capture.maximum - capture.minimum);
	dynamic_signal = (expected_amplitude_mvpp > 0U) &&
		((waveform == WAVEFORM_TYPE_SINE) ||
		 ((duty_percent > 0U) && (duty_percent < 100U)));

	if (dynamic_signal &&
		(((uint32_t)capture.maximum - capture.minimum) >=
		 SELF_TEST_MIN_DYNAMIC_SPAN))
	{
		if (!capture_frequency(capture.minimum, capture.maximum, capture_ms,
							   &measured_frequency_hz, &crossing_count))
		{
			monitor_adc_stop();
			diagnostic_uart_write("resultado=FALHA | erro durante captura\r\n");
			self_test_unlock();
			return false;
		}
	}
	monitor_adc_stop();

	write_measurement("minimo=", adc_code_to_mv(capture.minimum), " mV | ");
	write_measurement("maximo=", adc_code_to_mv(capture.maximum), " mV | ");
	write_measurement("media=", adc_code_to_mv(capture.average), " mV\r\n");
	write_measurement("vpp_PA4=", measured_amplitude_mvpp, " mV | ");
	write_measurement("amostras=", capture.sample_count, "\r\n");

	if (!dynamic_signal)
	{
		const uint32_t span = (uint32_t)capture.maximum - capture.minimum;
		const bool static_ok = span <= SELF_TEST_STATIC_SPAN_LIMIT;

		diagnostic_uart_write("sinal estatico; frequencia nao se aplica\r\n");
		diagnostic_uart_write(static_ok ?
			"resultado=OK\r\n" : "resultado=FALHA | saida instavel\r\n");
		self_test_unlock();
		return static_ok;
	}

	write_measurement("frequencia_medida=", measured_frequency_hz, " Hz | ");
	write_measurement("cruzamentos=", crossing_count, "\r\n");
	amplitude_tolerance_mv = (expected_amplitude_mvpp / 5U) + 30U;
	frequency_tolerance_hz = (expected_frequency_hz / 20U) + 2U;
	amplitude_ok = value_within_tolerance(measured_amplitude_mvpp,
		expected_amplitude_mvpp, amplitude_tolerance_mv);
	frequency_ok = value_within_tolerance(measured_frequency_hz,
		expected_frequency_hz, frequency_tolerance_hz);

	result_ok = amplitude_ok && frequency_ok;
	diagnostic_uart_write(result_ok ?
		"resultado=OK\r\n" : "resultado=FALHA | confira amplitude/frequencia\r\n");
	self_test_unlock();
	return result_ok;
}
