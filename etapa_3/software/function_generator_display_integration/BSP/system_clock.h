#ifndef SYSTEM_CLOCK_H
#define SYSTEM_CLOCK_H

#include <stdbool.h>

// frequencia principal do sistema

#define SYSTEM_CLOCK_HZ 180000000UL

bool system_clock_configure_180mhz(void);

#endif
