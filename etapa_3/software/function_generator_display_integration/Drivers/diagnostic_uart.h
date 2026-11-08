#ifndef DIAGNOSTIC_UART_H
#define DIAGNOSTIC_UART_H

#include <stdbool.h>
#include <stdint.h>

void diagnostic_uart_init(void);
void diagnostic_uart_write(const char *text);
void diagnostic_uart_write_u32(uint32_t value);
bool diagnostic_uart_try_read(char *character);

#endif
