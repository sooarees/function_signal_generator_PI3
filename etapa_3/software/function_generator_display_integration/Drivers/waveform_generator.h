#ifndef WAVEFORM_GENERATOR_H
#define WAVEFORM_GENERATOR_H

#include <stdbool.h>
#include <stdint.h>

// limites eletricos do gerador

#define WAVEFORM_GENERATOR_MIN_FREQUENCY_HZ 1U
#define WAVEFORM_GENERATOR_MAX_FREQUENCY_HZ 20000U
#define WAVEFORM_GENERATOR_MAX_AMPLITUDE_MVPP 2800U
#define WAVEFORM_GENERATOR_MIN_DUTY_PERCENT 0U
#define WAVEFORM_GENERATOR_MAX_DUTY_PERCENT 100U

// formas produzidas diretamente pelo dac

typedef enum
{
	WAVEFORM_TYPE_SINE = 0,
	WAVEFORM_TYPE_SQUARE
} waveform_type_t;

// controle do gerador

bool waveform_generator_start(waveform_type_t type,
							  uint32_t frequency_hz,
							  uint32_t amplitude_mvpp,
							  uint32_t duty_percent);
void waveform_generator_stop(void);
bool waveform_generator_set_type(waveform_type_t type);
bool waveform_generator_set_frequency(uint32_t frequency_hz);
bool waveform_generator_set_amplitude(uint32_t amplitude_mvpp);
bool waveform_generator_set_duty(uint32_t duty_percent);
waveform_type_t waveform_generator_get_type(void);
uint32_t waveform_generator_get_frequency(void);
uint32_t waveform_generator_get_amplitude(void);
uint32_t waveform_generator_get_duty(void);
uint32_t waveform_generator_get_sample_count(void);

#endif
