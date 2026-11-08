#include "FreeRTOS.h"
#include "task.h"

#include <stdbool.h>
#include <stdint.h>

#include "analog_self_test.h"
#include "diagnostic_uart.h"
#include "serial_control_task.h"
#include "signal_control.h"

#define SERIAL_COMMAND_BUFFER_SIZE 32U

// tratamento do texto recebido

static bool text_equals(const char *left, const char *right)
{
	while ((*left != '\0') && (*right != '\0'))
	{
		if (*left != *right)
		{
			return false;
		}
		left++;
		right++;
	}

	return (*left == '\0') && (*right == '\0');
}

static char *trim_command(char *command)
{
	char *start = command;
	char *end;

	while (*start == ' ')
	{
		start++;
	}

	end = start;
	while (*end != '\0')
	{
		end++;
	}
	while ((end > start) && (end[-1] == ' '))
	{
		end--;
	}
	*end = '\0';
	return start;
}

static void write_command_help(void)
{
	diagnostic_uart_write(
		"comandos de diagnostico: status | selftest\r\n");
}

// comandos de diagnostico

static void process_command(char *command)
{
	char *trimmed = trim_command(command);

	if (text_equals(trimmed, "status"))
	{
		if (!signal_control_log_settings())
		{
			diagnostic_uart_write("erro: controle ainda nao inicializado\r\n");
		}
		return;
	}

	if (text_equals(trimmed, "selftest"))
	{
		(void)analog_self_test_run();
		return;
	}

	diagnostic_uart_write("erro: ");
	write_command_help();
}

// leitura da serial

void serial_control_task(void *argument)
{
	char command_buffer[SERIAL_COMMAND_BUFFER_SIZE];
	uint32_t command_length = 0U;
	char character;

	(void)argument;
	write_command_help();

	for (;;)
	{
		while (diagnostic_uart_try_read(&character))
		{
			if ((character == '\r') || (character == '\n'))
			{
				if (command_length > 0U)
				{
					command_buffer[command_length] = '\0';
					process_command(command_buffer);
					command_length = 0U;
				}
			}
			else if ((character == '\b') || (character == 0x7F))
			{
				if (command_length > 0U)
				{
					command_length--;
				}
			}
			else if ((character >= 32) && (character <= 126))
			{
				if (command_length < (SERIAL_COMMAND_BUFFER_SIZE - 1U))
				{
					if ((character >= 'A') && (character <= 'Z'))
					{
						character = (char)(character + ('a' - 'A'));
					}
					command_buffer[command_length] = character;
					command_length++;
				}
				else
				{
					command_length = 0U;
					diagnostic_uart_write("erro: comando muito longo\r\n");
				}
			}
		}

		vTaskDelay(pdMS_TO_TICKS(1U));
	}
}
