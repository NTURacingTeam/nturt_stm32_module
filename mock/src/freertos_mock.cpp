#include "mock/freertos_mock.hpp"

extern "C" {
// freertos include
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
}

// gtest include
#include "cmock/cmock.h"

FreertosMock::FreertosMock() {}

FreertosMock::~FreertosMock() {}

#if 0

/* port functions ------------------------------------------------------------*/
CMOCK_MOCK_FUNCTION(FreertosMock, void, vPortYield, ());

CMOCK_MOCK_FUNCTION(FreertosMock, portBASE_TYPE, xPortSetInterruptMask, ());

CMOCK_MOCK_FUNCTION(FreertosMock, void, vPortClearInterruptMask,
                    (portBASE_TYPE));

CMOCK_MOCK_FUNCTION(FreertosMock, void, vPortDisableInterrupts, ());

CMOCK_MOCK_FUNCTION(FreertosMock, void, vPortEnableInterrupts, ());

CMOCK_MOCK_FUNCTION(FreertosMock, void, vTaskEnterCritical, ());

CMOCK_MOCK_FUNCTION(FreertosMock, void, vTaskExitCritical, ());

/* schedular management functions --------------------------------------------*/
CMOCK_MOCK_FUNCTION(FreertosMock, void, vTaskSuspendAll, ());

CMOCK_MOCK_FUNCTION(FreertosMock, BaseType_t, xTaskResumeAll, ());

CMOCK_MOCK_FUNCTION(FreertosMock, TickType_t, xTaskGetTickCount, ());

#endif

/* task management functions -------------------------------------------------*/
CMOCK_MOCK_FUNCTION(FreertosMock, BaseType_t, xTaskCreate,
                    (TaskFunction_t, const char* const,
                     const configSTACK_DEPTH_TYPE, void* const, UBaseType_t,
                     TaskHandle_t* const));

CMOCK_MOCK_FUNCTION(FreertosMock, TaskHandle_t, xTaskCreateStatic,
                    (TaskFunction_t, const char* const, const uint32_t,
                     void* const, UBaseType_t, StackType_t* const,
                     StaticTask_t* const));

CMOCK_MOCK_FUNCTION(FreertosMock, void, vTaskDelay, (const TickType_t));

CMOCK_MOCK_FUNCTION(FreertosMock, BaseType_t, xTaskDelayUntil,
                    (TickType_t* const, const TickType_t));

CMOCK_MOCK_FUNCTION(FreertosMock, void, vTaskSuspend, (TaskHandle_t));

CMOCK_MOCK_FUNCTION(FreertosMock, void, vTaskResume, (TaskHandle_t));

/* timer management functions ------------------------------------------------*/
CMOCK_MOCK_FUNCTION(FreertosMock, BaseType_t, xTimerPendFunctionCallFromISR,
                    (PendedFunction_t, void*, uint32_t, BaseType_t*));

CMOCK_MOCK_FUNCTION(FreertosMock, BaseType_t, xTimerGenericCommand,
                    (TimerHandle_t, const BaseType_t, const TickType_t,
                     BaseType_t* const, const TickType_t));

/* task nofity functions -----------------------------------------------------*/
CMOCK_MOCK_FUNCTION(FreertosMock, BaseType_t, xTaskGenericNotify,
                    (TaskHandle_t, uint32_t, eNotifyAction, uint32_t*));

CMOCK_MOCK_FUNCTION(FreertosMock, BaseType_t, xTaskGenericNotifyFromISR,
                    (TaskHandle_t, uint32_t, eNotifyAction, uint32_t*,
                     BaseType_t*));

CMOCK_MOCK_FUNCTION(FreertosMock, BaseType_t, xTaskNotifyWait,
                    (UBaseType_t, uint32_t, uint32_t*, TickType_t));

/* event groups management functions -----------------------------------------*/
CMOCK_MOCK_FUNCTION(FreertosMock, EventBits_t, xEventGroupWaitBits,
                    (EventGroupHandle_t, const EventBits_t, const BaseType_t,
                     const BaseType_t, TickType_t));

CMOCK_MOCK_FUNCTION(FreertosMock, EventBits_t, xEventGroupClearBits,
                    (EventGroupHandle_t, const EventBits_t));

CMOCK_MOCK_FUNCTION(FreertosMock, EventBits_t, xEventGroupSetBits,
                    (EventGroupHandle_t, const EventBits_t));

CMOCK_MOCK_FUNCTION(FreertosMock, EventBits_t, xEventGroupSync,
                    (EventGroupHandle_t, const EventBits_t, const EventBits_t,
                     TickType_t));

/* semaphore management functions --------------------------------------------*/
CMOCK_MOCK_FUNCTION(FreertosMock, QueueHandle_t, xQueueCreateMutexStatic,
                    (const uint8_t, StaticQueue_t*));

CMOCK_MOCK_FUNCTION(FreertosMock, BaseType_t, xQueueSemaphoreTake,
                    (QueueHandle_t, TickType_t));

/* queue management functions ------------------------------------------------*/
CMOCK_MOCK_FUNCTION(FreertosMock, BaseType_t, xQueueGenericSend,
                    (QueueHandle_t, const void* const, TickType_t,
                     const BaseType_t));

CMOCK_MOCK_FUNCTION(FreertosMock, BaseType_t, xQueuePeek,
                    (QueueHandle_t, void* const, TickType_t));

CMOCK_MOCK_FUNCTION(FreertosMock, BaseType_t, xQueueReceive,
                    (QueueHandle_t, void* const, TickType_t));
