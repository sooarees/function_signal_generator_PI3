#include "FreeRTOS.h"
#include "task.h"

#include "signal_control.h"
#include "signal_control_task.h"
#include "serial_control_task.h"
#include "system_clock.h"
#include "ui_task.h"

// tamanhos das pilhas

#define SIGNAL_CONTROL_TASK_STACK_WORDS (configMINIMAL_STACK_SIZE * 3U)
#define SERIAL_CONTROL_TASK_STACK_WORDS (configMINIMAL_STACK_SIZE * 3U)
#define UI_TASK_STACK_WORDS (configMINIMAL_STACK_SIZE * 3U)

// prioridades das tarefas

#define SIGNAL_CONTROL_TASK_PRIORITY 5U
#define SERIAL_CONTROL_TASK_PRIORITY 4U
#define UI_TASK_PRIORITY (tskIDLE_PRIORITY + 1U)

// tratamento de falhas

static void enter_safe_failure_state(void)
{
	signal_control_emergency_stop();
	taskDISABLE_INTERRUPTS();
	for (;;)
	{
	}
}

void vApplicationMallocFailedHook(void)
{
	enter_safe_failure_state();
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
	(void)task;
	(void)task_name;
	enter_safe_failure_state();
}

void vAssertCalled(const char *file, int line)
{
	(void)file;
	(void)line;
	enter_safe_failure_state();
}

// inicio do programa

int main(void)
{
	// clock principal

	if (!system_clock_configure_180mhz())
	{
		vAssertCalled(__FILE__, __LINE__);
	}

	// criacao das tarefas

	if (xTaskCreate(signal_control_task,
					"SignalControl",
					SIGNAL_CONTROL_TASK_STACK_WORDS,
					NULL,
					SIGNAL_CONTROL_TASK_PRIORITY,
					NULL) != pdPASS)
	{
		vApplicationMallocFailedHook();
	}

	if (xTaskCreate(serial_control_task,
					"SerialControl",
					SERIAL_CONTROL_TASK_STACK_WORDS,
					NULL,
					SERIAL_CONTROL_TASK_PRIORITY,
					NULL) != pdPASS)
	{
		vApplicationMallocFailedHook();
	}

	if (xTaskCreate(ui_task,
					"Interface",
					UI_TASK_STACK_WORDS,
					NULL,
					UI_TASK_PRIORITY,
					NULL) != pdPASS)
	{
		vApplicationMallocFailedHook();
	}

	// inicio do freertos

	vTaskStartScheduler();

	// o programa so chega aqui se o scheduler falhar

	for (;;)
	{
	}
}
