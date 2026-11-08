#include "FreeRTOS.h"
#include "task.h"

#include "display.h"
#include "touch.h"
#include "ui.h"
#include "ui_task.h"

// inicializacao e leitura da interface

void ui_task(void *argument)
{
	bool ui_started;

	(void)argument;
	display_init();
	touch_init();
	ui_started = ui_init();
	configASSERT(ui_started);
	if (!ui_started)
	{
		for (;;)
		{
			vTaskDelay(portMAX_DELAY);
		}
	}

	// atualizacao periodica do touch

	for (;;)
	{
		touch_point_t point;
		const uint32_t now_ms =
			(uint32_t)xTaskGetTickCount() * (uint32_t)portTICK_PERIOD_MS;

		if (touch_read(&point))
		{
			(void)ui_process_touch(&point, now_ms);
		}

		vTaskDelay(pdMS_TO_TICKS(10U));
	}
}
