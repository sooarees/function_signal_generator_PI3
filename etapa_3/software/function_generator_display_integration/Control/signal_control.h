#ifndef SIGNAL_CONTROL_H
#define SIGNAL_CONTROL_H

#include <stdbool.h>
#include <stdint.h>

// formas de onda escolhidas pela interface

typedef enum
{
	SIGNAL_WAVEFORM_SINE = 0,
	SIGNAL_WAVEFORM_SQUARE,
	SIGNAL_WAVEFORM_TRIANGLE,
	SIGNAL_WAVEFORM_SAW
} signal_waveform_t;

// configuracao compartilhada entre as tarefas

typedef struct
{
	signal_waveform_t waveform;
	uint32_t frequency_hz;
	float amplitude_vpp;
	float offset_v;
	uint32_t duty_percent;
} signal_control_settings_t;

// controle da saida

bool signal_control_init(void);
void signal_control_emergency_stop(void);
bool signal_control_set_waveform(signal_waveform_t waveform);
bool signal_control_set_frequency(uint32_t frequency_hz);
bool signal_control_set_amplitude(float amplitude_vpp);
bool signal_control_set_offset(float offset_v);
bool signal_control_set_duty(uint32_t duty_percent);
bool signal_control_get_settings(signal_control_settings_t *settings);
bool signal_control_log_settings(void);

#endif
