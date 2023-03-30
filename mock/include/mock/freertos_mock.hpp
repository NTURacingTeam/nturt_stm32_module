#ifndef FREERTOS_MOCK_HPP
#define FREERTOS_MOCK_HPP

// glibc include
#include <stdint.h>

// gtest include
#include "cmock/cmock.h"

// freertos include
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

/// @brief Class for mocking freertos function using google test framework.
class FreeRTOS_Mock : public CMockMocker<FreeRTOS_Mock> {
 public:
  FreeRTOS_Mock();

  ~FreeRTOS_Mock();

  /* port functions
   * -----------------------------------------------------------*/
  CMOCK_MOCK_METHOD(void, vPortYield, ());

  CMOCK_MOCK_METHOD(portBASE_TYPE, xPortSetInterruptMask, ());

  CMOCK_MOCK_METHOD(void, vPortClearInterruptMask, (portBASE_TYPE));

  CMOCK_MOCK_METHOD(void, vPortDisableInterrupts, ());

  CMOCK_MOCK_METHOD(void, vPortEnableInterrupts, ());

  CMOCK_MOCK_METHOD(void, vTaskEnterCritical, ());

  CMOCK_MOCK_METHOD(void, vTaskExitCritical, ());

  CMOCK_MOCK_METHOD(BaseType_t, xPortIsInsideInterrupt, ());

  /* schedular management functions ------------------------------------------*/
  CMOCK_MOCK_METHOD(void, vTaskSuspendAll, ());

  CMOCK_MOCK_METHOD(BaseType_t, xTaskResumeAll, ());

  CMOCK_MOCK_METHOD(TickType_t, xTaskGetTickCount, ());

  /* task management functions -----------------------------------------------*/
  CMOCK_MOCK_METHOD(BaseType_t, xTaskCreate,
                    (TaskFunction_t, const char* const,
                     const configSTACK_DEPTH_TYPE, void* const, UBaseType_t,
                     TaskHandle_t* const));

  CMOCK_MOCK_METHOD(TaskHandle_t, xTaskCreateStatic,
                    (TaskFunction_t, const char* const, const uint32_t,
                     void* const, UBaseType_t, StackType_t* const,
                     StaticTask_t* const));

  CMOCK_MOCK_METHOD(void, vTaskDelay, (const TickType_t));

  CMOCK_MOCK_METHOD(BaseType_t, xTaskDelayUntil,
                    (TickType_t* const, const TickType_t));

  CMOCK_MOCK_METHOD(void, vTaskSuspend, (TaskHandle_t));

  CMOCK_MOCK_METHOD(void, vTaskResume, (TaskHandle_t));

  /* timer management functions ----------------------------------------------*/
  CMOCK_MOCK_METHOD(BaseType_t, xTimerPendFunctionCallFromISR,
                    (PendedFunction_t, void*, uint32_t, BaseType_t*));

  CMOCK_MOCK_METHOD(BaseType_t, xTimerGenericCommand,
                    (TimerHandle_t, const BaseType_t, const TickType_t,
                     BaseType_t* const, const TickType_t));

  /* task nofity functions ---------------------------------------------------*/
  CMOCK_MOCK_METHOD(BaseType_t, xTaskNotify,
                    (TaskHandle_t, uint32_t, eNotifyAction));

  CMOCK_MOCK_METHOD(BaseType_t, xTaskNotifyFromISR,
                    (TaskHandle_t, uint32_t, eNotifyAction, BaseType_t*));

  CMOCK_MOCK_METHOD(BaseType_t, xTaskNotifyWait,
                    (UBaseType_t, uint32_t, uint32_t*, TickType_t));

  /* event groups management functions ---------------------------------------*/
  CMOCK_MOCK_METHOD(EventBits_t, xEventGroupWaitBits,
                    (EventGroupHandle_t, const EventBits_t, const BaseType_t,
                     const BaseType_t, TickType_t));

  CMOCK_MOCK_METHOD(EventBits_t, xEventGroupClearBits,
                    (EventGroupHandle_t, const EventBits_t));

  CMOCK_MOCK_METHOD(EventBits_t, xEventGroupSetBits,
                    (EventGroupHandle_t, const EventBits_t));

  CMOCK_MOCK_METHOD(EventBits_t, xEventGroupSync,
                    (EventGroupHandle_t, const EventBits_t, const EventBits_t,
                     TickType_t));

  /* semaphore management functions ------------------------------------------*/
  CMOCK_MOCK_METHOD(SemaphoreHandle_t, xSemaphoreCreateMutexStatic,
                    (StaticSemaphore_t*));

  CMOCK_MOCK_METHOD(BaseType_t, xSemaphoreTake,
                    (SemaphoreHandle_t, TickType_t));

  CMOCK_MOCK_METHOD(BaseType_t, xSemaphoreGive, (SemaphoreHandle_t));

  /* queue management functions ----------------------------------------------*/
  CMOCK_MOCK_METHOD(BaseType_t, xQueueGenericSend,
                    (QueueHandle_t, const void* const, TickType_t,
                     const BaseType_t));

  CMOCK_MOCK_METHOD(BaseType_t, xQueuePeek,
                    (QueueHandle_t, void* const, TickType_t));

  CMOCK_MOCK_METHOD(BaseType_t, xQueueReceive,
                    (QueueHandle_t, void* const, TickType_t));
};

#endif  // FREERTOS_MOCK_HPP
