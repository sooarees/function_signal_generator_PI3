#include "signal_control.h"

#include <stddef.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "diagnostic_uart.h"
#include "signal_control_config.h"
#include "waveform_generator.h"

// configuracao interna

#define FIXED_DUTY_FOR_EXTERNAL_WAVEFORMS 50U

static SemaphoreHandle_t control_mutex;
static signal_control_settings_t current_settings;

// validacao e traducao das formas de onda

static bool signal_waveform_is_valid(signal_waveform_t waveform)
{
	return (waveform == SIGNAL_WAVEFORM_SINE) ||
		   (waveform == SIGNAL_WAVEFORM_SQUARE) ||
		   (waveform == SIGNAL_WAVEFORM_TRIANGLE) ||
		   (waveform == SIGNAL_WAVEFORM_SAW);
}

static const char *signal_waveform_name(signal_waveform_t waveform)
{
	switch (waveform)
	{
	case SIGNAL_WAVEFORM_SQUARE:
		return "quadrada";
	case SIGNAL_WAVEFORM_TRIANGLE:
		return "triangular";
	case SIGNAL_WAVEFORM_SAW:
		return "dente_de_serra";
	case SIGNAL_WAVEFORM_SINE:
	default:
		return "senoide";
	}
}

static waveform_type_t effective_driver_waveform(signal_waveform_t waveform)
{
	return (waveform == SIGNAL_WAVEFORM_SINE) ?
			   WAVEFORM_TYPE_SINE : WAVEFORM_TYPE_SQUARE;
}

static uint32_t effective_driver_duty(signal_waveform_t waveform,
									  uint32_t configured_duty_percent)
{
	if ((waveform == SIGNAL_WAVEFORM_TRIANGLE) ||
		(waveform == SIGNAL_WAVEFORM_SAW))
	{
		return FIXED_DUTY_FOR_EXTERNAL_WAVEFORMS;
	}

	return configured_duty_percent;
}

// saida de diagnostico

static void log_decimal_hundredths(uint32_t value_hundredths)
{
	diagnostic_uart_write_u32(value_hundredths / 100U);
	diagnostic_uart_write(".");
	if ((value_hundredths % 100U) < 10U)
	{
		diagnostic_uart_write("0");
	}
	diagnostic_uart_write_u32(value_hundredths % 100U);
}

static void log_signed_decimal_hundredths(float value)
{
	uint32_t absolute_hundredths;

	if (value < 0.0f)
	{
		diagnostic_uart_write("-");
		value = -value;
	}
	else
	{
		diagnostic_uart_write("+");
	}

	absolute_hundredths = (uint32_t)((value * 100.0f) + 0.5f);
	log_decimal_hundredths(absolute_hundredths);
}

static void log_current_settings(void)
{
	const waveform_type_t driver_waveform =
		effective_driver_waveform(current_settings.waveform);
	const uint32_t driver_duty =
		effective_driver_duty(current_settings.waveform,
							  current_settings.duty_percent);
	const uint32_t amplitude_hundredths =
		(uint32_t)((current_settings.amplitude_vpp * 100.0f) + 0.5f);

	diagnostic_uart_write("onda=");
	diagnostic_uart_write(signal_waveform_name(current_settings.waveform));
	diagnostic_uart_write(" | geracao_dac=");
	diagnostic_uart_write((driver_waveform == WAVEFORM_TYPE_SINE) ?
						  "senoide" : "quadrada");
	diagnostic_uart_write(" | frequencia=");
	diagnostic_uart_write_u32(current_settings.frequency_hz);
	diagnostic_uart_write(" Hz");
	if (driver_waveform == WAVEFORM_TYPE_SINE)
	{
		diagnostic_uart_write(" | pontos_senoide=");
		diagnostic_uart_write_u32(waveform_generator_get_sample_count());
	}
	diagnostic_uart_write(" | amplitude=");
	log_decimal_hundredths(amplitude_hundredths);
	diagnostic_uart_write(" Vpp | offset=");
	log_signed_decimal_hundredths(current_settings.offset_v);
	diagnostic_uart_write(" V (guardado) | duty_configurado=");
	diagnostic_uart_write_u32(current_settings.duty_percent);
	diagnostic_uart_write("%");
	if (driver_waveform == WAVEFORM_TYPE_SQUARE)
	{
		diagnostic_uart_write(" | duty_dac=");
		diagnostic_uart_write_u32(driver_duty);
		diagnostic_uart_write("%");
	}
	diagnostic_uart_write("\r\n");
}

static bool settings_are_valid(signal_waveform_t waveform,
							   uint32_t frequency_hz,
							   float amplitude_vpp,
							   float offset_v,
							   uint32_t duty_percent)
{
	return signal_waveform_is_valid(waveform) &&
		   (frequency_hz >= SIGNAL_MIN_FREQUENCY_HZ) &&
		   (frequency_hz <= SIGNAL_MAX_FREQUENCY_HZ) &&
		   (amplitude_vpp >= SIGNAL_MIN_AMPLITUDE_VPP) &&
		   (amplitude_vpp <= SIGNAL_MAX_AMPLITUDE_VPP) &&
		   (offset_v >= SIGNAL_MIN_OFFSET_V) &&
		   (offset_v <= SIGNAL_MAX_OFFSET_V) &&
		   (duty_percent >= SIGNAL_MIN_DUTY_PERCENT) &&
		   (duty_percent <= SIGNAL_MAX_DUTY_PERCENT);
}

// conversao da amplitude externa para a saida do dac

static uint32_t output_vpp_to_dac_mvpp(float output_amplitude_vpp)
{
	return (uint32_t)(((output_amplitude_vpp /
						 SIGNAL_OUTPUT_AMPLIFIER_GAIN) * 1000.0f) + 0.5f);
}

static bool take_control_mutex(void)
{
	return (control_mutex != NULL) &&
		   (xSemaphoreTake(control_mutex, portMAX_DELAY) == pdTRUE);
}

// aplicacao segura da forma de onda

static bool apply_waveform_selection(signal_waveform_t waveform,
									 uint32_t configured_duty_percent)
{
	const waveform_type_t previous_type = waveform_generator_get_type();
	const uint32_t previous_duty = waveform_generator_get_duty();
	const waveform_type_t requested_type = effective_driver_waveform(waveform);
	const uint32_t requested_duty =
		effective_driver_duty(waveform, configured_duty_percent);

	if (requested_type == WAVEFORM_TYPE_SINE)
	{
		// troca primeiro para senoide para nao aplicar outro duty na quadrada
		// nem por um instante durante a mudanca.
		if (!waveform_generator_set_type(requested_type) ||
			!waveform_generator_set_duty(requested_duty))
		{
			(void)waveform_generator_set_type(previous_type);
			(void)waveform_generator_set_duty(previous_duty);
			return false;
		}
	}
	else
	{
		// prepara o duty antes de habilitar a geracao quadrada.
		if (!waveform_generator_set_duty(requested_duty) ||
			!waveform_generator_set_type(requested_type))
		{
			(void)waveform_generator_set_type(previous_type);
			(void)waveform_generator_set_duty(previous_duty);
			return false;
		}
	}

	return true;
}

// inicializacao do estado central e da saida

bool signal_control_init(void)
{
	uint32_t dac_amplitude_mvpp;
	const waveform_type_t driver_waveform =
		effective_driver_waveform(SIGNAL_DEFAULT_WAVEFORM);
	const uint32_t driver_duty =
		effective_driver_duty(SIGNAL_DEFAULT_WAVEFORM,
							  SIGNAL_DEFAULT_DUTY_PERCENT);

	if ((control_mutex != NULL) ||
		!settings_are_valid(SIGNAL_DEFAULT_WAVEFORM,
							SIGNAL_DEFAULT_FREQUENCY_HZ,
							SIGNAL_DEFAULT_AMPLITUDE_VPP,
							SIGNAL_DEFAULT_OFFSET_V,
							SIGNAL_DEFAULT_DUTY_PERCENT))
	{
		return false;
	}

	control_mutex = xSemaphoreCreateMutex();
	if (control_mutex == NULL)
	{
		return false;
	}

	dac_amplitude_mvpp = output_vpp_to_dac_mvpp(SIGNAL_DEFAULT_AMPLITUDE_VPP);
	if (!waveform_generator_start(driver_waveform,
								  SIGNAL_DEFAULT_FREQUENCY_HZ,
								  dac_amplitude_mvpp,
								  driver_duty))
	{
		vSemaphoreDelete(control_mutex);
		control_mutex = NULL;
		return false;
	}

	current_settings.waveform = SIGNAL_DEFAULT_WAVEFORM;
	current_settings.frequency_hz = SIGNAL_DEFAULT_FREQUENCY_HZ;
	current_settings.amplitude_vpp = SIGNAL_DEFAULT_AMPLITUDE_VPP;
	current_settings.offset_v = SIGNAL_DEFAULT_OFFSET_V;
	current_settings.duty_percent = SIGNAL_DEFAULT_DUTY_PERCENT;
	diagnostic_uart_init();
	log_current_settings();
	return true;
}

// parada imediata usada pelos tratamentos de falha

void signal_control_emergency_stop(void)
{
	// nao usa mutex porque tambem precisa funcionar durante falhas do freertos.
	waveform_generator_stop();
}

// escolha logica da forma e traducao para o driver fisico

bool signal_control_set_waveform(signal_waveform_t waveform)
{
	bool updated;

	if (!signal_waveform_is_valid(waveform) || !take_control_mutex())
	{
		return false;
	}

	updated = apply_waveform_selection(waveform,
								   current_settings.duty_percent);
	if (updated)
	{
		current_settings.waveform = waveform;
		log_current_settings();
	}

	xSemaphoreGive(control_mutex);
	return updated;
}

// frequencia compartilhada pelas quatro escolhas

bool signal_control_set_frequency(uint32_t frequency_hz)
{
	bool updated;

	if ((frequency_hz < SIGNAL_MIN_FREQUENCY_HZ) ||
		(frequency_hz > SIGNAL_MAX_FREQUENCY_HZ) ||
		!take_control_mutex())
	{
		return false;
	}

	updated = waveform_generator_set_frequency(frequency_hz);
	if (updated)
	{
		current_settings.frequency_hz = frequency_hz;
		log_current_settings();
	}

	xSemaphoreGive(control_mutex);
	return updated;
}

// amplitude compartilhada pelas quatro escolhas

bool signal_control_set_amplitude(float amplitude_vpp)
{
	uint32_t dac_amplitude_mvpp;
	bool updated;

	if (!((amplitude_vpp >= SIGNAL_MIN_AMPLITUDE_VPP) &&
		  (amplitude_vpp <= SIGNAL_MAX_AMPLITUDE_VPP)) ||
		!take_control_mutex())
	{
		return false;
	}

	dac_amplitude_mvpp = output_vpp_to_dac_mvpp(amplitude_vpp);
	updated = waveform_generator_set_amplitude(dac_amplitude_mvpp);
	if (updated)
	{
		current_settings.amplitude_vpp = amplitude_vpp;
		log_current_settings();
	}

	xSemaphoreGive(control_mutex);
	return updated;
}

// offset guardado para a integracao futura, sem efeito eletrico por enquanto

bool signal_control_set_offset(float offset_v)
{
	if (!((offset_v >= SIGNAL_MIN_OFFSET_V) &&
		  (offset_v <= SIGNAL_MAX_OFFSET_V)) ||
		!take_control_mutex())
	{
		return false;
	}

	current_settings.offset_v = offset_v;
	log_current_settings();
	xSemaphoreGive(control_mutex);
	return true;
}

// duty configurado pertence a quadrada; triangular e dente usam 50% no dac

bool signal_control_set_duty(uint32_t duty_percent)
{
	bool updated = true;

	if ((duty_percent < SIGNAL_MIN_DUTY_PERCENT) ||
		(duty_percent > SIGNAL_MAX_DUTY_PERCENT) ||
		!take_control_mutex())
	{
		return false;
	}

	if (current_settings.waveform == SIGNAL_WAVEFORM_SQUARE)
	{
		updated = waveform_generator_set_duty(duty_percent);
	}

	if (updated)
	{
		current_settings.duty_percent = duty_percent;
		log_current_settings();
	}

	xSemaphoreGive(control_mutex);
	return updated;
}

// leitura do estado atual

bool signal_control_get_settings(signal_control_settings_t *settings)
{
	if ((settings == NULL) || !take_control_mutex())
	{
		return false;
	}

	*settings = current_settings;
	xSemaphoreGive(control_mutex);
	return true;
}

bool signal_control_log_settings(void)
{
	if (!take_control_mutex())
	{
		return false;
	}

	log_current_settings();
	xSemaphoreGive(control_mutex);
	return true;
}
