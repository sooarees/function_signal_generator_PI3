#ifndef SIGNAL_CONTROL_CONFIG_H
#define SIGNAL_CONTROL_CONFIG_H

#include "signal_control.h"

// configuracao inicial

#define SIGNAL_DEFAULT_WAVEFORM           SIGNAL_WAVEFORM_SINE //SIGNAL_WAVEFORM_SINE / SIGNAL_WAVEFORM_SQUARE
#define SIGNAL_DEFAULT_FREQUENCY_HZ       20000U
#define SIGNAL_DEFAULT_AMPLITUDE_VPP      9.00f
#define SIGNAL_DEFAULT_OFFSET_V           0.00f
#define SIGNAL_DEFAULT_DUTY_PERCENT       50U

// limites compartilhados pelo controle

#define SIGNAL_MIN_FREQUENCY_HZ           1U
#define SIGNAL_MAX_FREQUENCY_HZ           20000U
#define SIGNAL_MIN_AMPLITUDE_VPP          0.00f
#define SIGNAL_MAX_AMPLITUDE_VPP          9.80f
#define SIGNAL_MIN_OFFSET_V               -5.00f
#define SIGNAL_MAX_OFFSET_V               5.00f
#define SIGNAL_MIN_DUTY_PERCENT           0U
#define SIGNAL_MAX_DUTY_PERCENT           100U
#define SIGNAL_OUTPUT_AMPLIFIER_GAIN      3.5f

#endif
