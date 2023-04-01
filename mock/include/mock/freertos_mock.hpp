#ifndef FREERTOS_MOCK_HPP
#define FREERTOS_MOCK_HPP

// stl include
#include <cstdint>

// gtest include
#include "cmock/cmock.h"

extern "C" {
// freertos include
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
}

/// @brief Class for mocking freertos function using google test framework.
class FreertosMock : public CMockMocker<FreertosMock> {
 public:
  FreertosMock();

  ~FreertosMock();

#if 0

  /* port functions ----------------------------------------------------------*/
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

#endif

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
  CMOCK_MOCK_METHOD(BaseType_t, xTaskGenericNotify,
                    (TaskHandle_t, uint32_t, eNotifyAction, uint32_t*));

  CMOCK_MOCK_METHOD(BaseType_t, xTaskGenericNotifyFromISR,
                    (TaskHandle_t, uint32_t, eNotifyAction, uint32_t*,
                     BaseType_t*));

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
  CMOCK_MOCK_METHOD(QueueHandle_t, xQueueCreateMutexStatic,
                    (const uint8_t, StaticQueue_t*));

  CMOCK_MOCK_METHOD(BaseType_t, xQueueSemaphoreTake,
                    (QueueHandle_t, TickType_t));

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
