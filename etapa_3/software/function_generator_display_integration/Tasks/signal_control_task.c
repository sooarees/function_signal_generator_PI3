#include "FreeRTOS.h"
#include "task.h"

#include "signal_control.h"
#include "signal_control_task.h"

// inicializacao do gerador

void signal_control_task(void *argument)
{
	bool control_started;

	(void)argument;

	control_started = signal_control_init();
	configASSERT(control_started);
	if (!control_started)
	{
		for (;;)
		{
			vTaskDelay(portMAX_DELAY);
		}
	}

	// o controle funciona por interrupcao e aguarda comandos das outras tarefas

	for (;;)
	{
		vTaskDelay(pdMS_TO_TICKS(1000U));
	}
}
