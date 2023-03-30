#ifndef MOCK_COMMON_HPP
#define MOCK_COMMON_HPP

// stl include
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

// gtest include
#include "cmock/cmock.h"

// freertos include
#include "FreeRTOS.h"
#include "task.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* type ----------------------------------------------------------------------*/
enum class FreertosSimulatorMode {
  COUNT_DOWN,
  CONTINUOUS,
};

/* macro ---------------------------------------------------------------------*/
#define SIMULATE_FREERTOS_CALL_ONCE(obj, handler, call) \
  EXPECT_CALL(obj, call).WillOnce(                      \
      ::testing::Invoke(&handler, &FreertosSimulator::call))

#define SIMULATE_FREERTOS_CALL(obj, handler, call) \
  EXPECT_CALL(obj, call).WillRepeatedly(           \
      ::testing::Invoke(&handler, &FreertosSimulator::call))

/* mock ----------------------------------------------------------------------*/
/// @brief Class for mocking common function using google test framework.
class CommonMock : public CMockMocker<CommonMock> {
 public:
  CommonMock();

  ~CommonMock();

  // CMOCK_MOCK_METHOD(void, __module_assert_fail,
  //                   (const char *, const char *, unsigned int, const char
  //                   *));
};

namespace mock {

/* notification --------------------------------------------------------------*/
class Notification {
 public:
  void wait();

  void notify_one();

  void notify_all();

 private:
  std::mutex m_;

  std::condition_variable cv_;

  bool done_ = false;
};

#if 0

/* freertos simulator --------------------------------------------------------*/
/// @brief Class for simulating freertos function in google test framework.
class FreertosSimulator {
 public:
  /**
   * @brief Constructor. When this overload is used, the freertos simulator is
   * in continuous mode.
   *
   */
  FreertosSimulator();

  /**
   * @brief Constructor. When this overload is used, the freertos simulator is
   * in countdown mode.
   *
   * @param[in] tick_to_run The number of ticks to run before the freertos
   * simulator stops.
   */
  FreertosSimulator(TickType_t tick_to_run);

  /**
   * @brief Function to start the freertos simulator, and it will start the
   * freertos task loop.
   *
   * @return None.
   */
  void start();

  /**
   * @brief Function to stop the freertos simulator, and it will stop the
   * freertos task loop.
   *
   * @return None.
   */
  void stop();

  /* schedular management functions ------------------------------------------*/
  /**
   * @brief Function to get the current tick count. The tick count start from 0
   * and is incremented by xTimeIncrement every time xTaskDelayUntil is called.
   *
   * @return The current simulated tick count.
   */
  TickType_t xTaskGetTickCount();

  /* task management functions -----------------------------------------------*/
  /**
   * @brief Function to simulate freertos xTaskCreateStatic function.
   *
   * @param[in] pxTaskCode Task code of the task.
   * @param[in] pcName Name of the task.
   * @param[in] ulStackDepth Not used in this simulator.
   * @param[in] pvParameters Parameter of the task code.
   * @param[in] uxPriority Not used in this simulator.eAction
   * @param[in] puxStackBuffer Not used in this simulator.
   * @param[in] pxTaskBuffer Not used in this simulator.
   * @return TaskHandle_t Simulated task handle for use in other freertos
   * functions.
   */
  TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
                                 const char *const pcName,
                                 const uint32_t ulStackDepth,
                                 void *const pvParameters,
                                 UBaseType_t uxPriority,
                                 StackType_t *const puxStackBuffer,
                                 StaticTask_t *const pxTaskBuffer);
  /**
   * @brief Function to simulate freertos xTaskDelayUntil function.
   *
   * @param[out] pxPreviousWakeTime The time at which the task was last
   * unblocked.
   * @param[in] xTimeIncrement How many ticks after the task will be unblocked.
   *
   * @return MODULE_END if the freertos simulator is going to stop, otherwise
   * pdTRUE for the task to keep going.
   * @note The original freertos function will return pdFASLE or pdTRUE to
   * indicate if the task was blocked or not, here it's taken advantage to
   * return MODULE_END to stop the loop in testing. MODULE will never have the
   * same value as pdFALSE or pdTRUE.
   */
  BaseType_t xTaskDelayUntil(TickType_t *const pxPreviousWakeTime,
                             const TickType_t xTimeIncrement);

  /* task nofity functions ---------------------------------------------------*/
  BaseType_t xTaskNotify(TaskHandle_t xTaskToNotify, uint32_t ulValue,
                         eNotifyAction eAction);

  BaseType_t xTaskNotifyFromISR(TaskHandle_t xTaskToNotify, uint32_t ulValue,
                                eNotifyAction eAction,
                                BaseType_t *pxHigherPriorityTaskWoken);

  BaseType_t xTaskNotifyWait(UBaseType_t ulBitsToClearOnEntry,
                             uint32_t ulBitsToClearOnExit,
                             uint32_t *pulNotificationValue,
                             TickType_t xTicksToWait);

 private:
  /* task --------------------------------------------------------------------*/
  const char *task_name_;

  std::function<void()> task_code_;

  std::thread task_thread_;

  /* task notify -------------------------------------------------------------*/
  uint32_t task_notify_value_;

  std::mutex task_notify_mutex_;

  std::condition_variable task_notify_cv_;

  bool task_notify_check_;

  /* kernel ------------------------------------------------------------------*/
  FreertosSimulatorMode mode_ = FreertosSimulatorMode::CONTINUOUS;

  TickType_t current_tick_ = 0;

  TickType_t tick_to_run_ = 0;

  std::atomic_bool should_stop_ = false;
};

#endif

}  // namespace mock

#endif  // MOCK_COMMON_HPP
