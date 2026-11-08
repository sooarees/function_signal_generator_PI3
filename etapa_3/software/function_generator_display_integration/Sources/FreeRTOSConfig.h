#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

#include "system_clock.h"

void vAssertCalled(const char *file, int line);

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configCPU_CLOCK_HZ                      SYSTEM_CLOCK_HZ
#define configTICK_RATE_HZ                      1000U
#define configMAX_PRIORITIES                    8U
#define configMINIMAL_STACK_SIZE                128U
#define configTOTAL_HEAP_SIZE                   (16U * 1024U)
#define configMAX_TASK_NAME_LEN                 16U
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TIME_SLICING                  1

#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            1
#define configCHECK_FOR_STACK_OVERFLOW          2

#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0

#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_QUEUE_SETS                    0
#define configQUEUE_REGISTRY_SIZE               0U

#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_TIMERS                        0
#define configUSE_EVENT_GROUPS                  0
#define configUSE_STREAM_BUFFERS                0
#define configUSE_CO_ROUTINES                   0

#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_NEWLIB_REENTRANT              0

#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskDelayUntil                 1
#define INCLUDE_vTaskDelete                     0
#define INCLUDE_vTaskSuspend                    0
#define INCLUDE_xTaskGetSchedulerState          0

#define configPRIO_BITS                         4U
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15U
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5U
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8U - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8U - configPRIO_BITS))

#define vPortSVCHandler                         SVC_Handler
#define xPortPendSVHandler                      PendSV_Handler
#define xPortSysTickHandler                     SysTick_Handler

#define configASSERT(condition)                 do { if ((condition) == 0) { vAssertCalled(__FILE__, __LINE__); } } while (0)

#endif
