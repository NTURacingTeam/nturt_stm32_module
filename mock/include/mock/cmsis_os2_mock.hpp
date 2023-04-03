#ifndef CMSIS_OS2_MOCK_HPP
#define CMSIS_OS2_MOCK_HPP

// stl include
#include <cstdint>

extern "C" {
// stm32 incldue
#include "cmsis_os2.h"
}

// gtest include
#include "cmock/cmock.h"

/// @brief Class for mocking cmsis rtos v2 function using google test framework.
class CmsisOs2Mock : public CMockMocker<CmsisOs2Mock> {
 public:
  CmsisOs2Mock();

  ~CmsisOs2Mock();

  /* kernel menagenent functions ---------------------------------------------*/
  CMOCK_MOCK_METHOD(uint32_t, osKernelGetTickCount, ());

  CMOCK_MOCK_METHOD(uint32_t, osKernelGetTickFreq, ());

  /* thread menagenent functions ---------------------------------------------*/
  CMOCK_MOCK_METHOD(osStatus_t, osThreadYield, ());

  CMOCK_MOCK_METHOD(osStatus_t, osThreadSuspend, (osThreadId_t));

  CMOCK_MOCK_METHOD(osStatus_t, osThreadResume, (osThreadId_t));

  /* generic wait functions --------------------------------------------------*/
  CMOCK_MOCK_METHOD(osStatus_t, osDelay, (uint32_t));

  CMOCK_MOCK_METHOD(osStatus_t, osDelayUntil, (uint32_t));

  /* thread flags functions --------------------------------------------------*/
  CMOCK_MOCK_METHOD(uint32_t, osThreadFlagsSet, (osThreadId_t, uint32_t));

  CMOCK_MOCK_METHOD(uint32_t, osThreadFlagsClear, (uint32_t));

  CMOCK_MOCK_METHOD(uint32_t, osThreadFlagsGet, ());

  CMOCK_MOCK_METHOD(uint32_t, osThreadFlagsWait,
                    (uint32_t, uint32_t, uint32_t));

  /* event flags management functions ----------------------------------------*/
  CMOCK_MOCK_METHOD(uint32_t, osEventFlagsSet, (osEventFlagsId_t, uint32_t));

  CMOCK_MOCK_METHOD(uint32_t, osEventFlagsClear, (osEventFlagsId_t, uint32_t));

  CMOCK_MOCK_METHOD(uint32_t, osEventFlagsGet, (osEventFlagsId_t));

  CMOCK_MOCK_METHOD(uint32_t, osEventFlagsWait,
                    (osEventFlagsId_t, uint32_t, uint32_t, uint32_t));

  /* mutex management functions ----------------------------------------------*/
  CMOCK_MOCK_METHOD(osStatus_t, osMutexAcquire, (osMutexId_t, uint32_t));

  CMOCK_MOCK_METHOD(osStatus_t, osMutexRelease, (osMutexId_t));

  /* queue management functions ----------------------------------------------*/
  CMOCK_MOCK_METHOD(osStatus_t, osMessageQueuePut,
                    (osMessageQueueId_t, const void *, uint8_t, uint32_t));

  CMOCK_MOCK_METHOD(osStatus_t, osMessageQueueGet,
                    (osMessageQueueId_t, void *, uint8_t *, uint32_t));
};

#endif  // CMSIS_OS2_MOCK_HPP
