#include "mock/mock_common.hpp"

// stl include
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

extern "C" {
// freertos include
#include "FreeRTOS.h"
#include "task.h"

// nturt include
#include "stm32_module/module_common.h"
}

// google test include
#include "gtest/gtest.h"

/* mock ----------------------------------------------------------------------*/
CommonMock::CommonMock() {}

CommonMock::~CommonMock() {}

CMOCK_MOCK_FUNCTION(CommonMock, void, __module_assert_fail,
                    (const char *, const char *, unsigned int, const char *));

namespace mock {

static void googletest_task(void *pvParameters) {
  int *test_result = (int *)pvParameters;

  *test_result = RUN_ALL_TESTS();
  vTaskEndScheduler();
}

int run_freertos_test(int *argc, char **argv) {
  ::testing::InitGoogleTest(argc, argv);

  int test_result;
  xTaskCreate(googletest_task, "googletest_task", PTHREAD_STACK_MIN,
              &test_result, 1, NULL);
  vTaskStartScheduler();

  return test_result;
}

/* notification --------------------------------------------------------------*/
void Notification::wait() {
  std::unique_lock lk(m_);
  cv_.wait(lk, [this] { return done_; });
  done_ = false;
}

void Notification::notify_one() {
  {
    std::unique_lock<std::mutex> lock(m_);
    done_ = true;
  }
  cv_.notify_one();
}

void Notification::notify_all() {
  {
    std::unique_lock<std::mutex> lock(m_);
    done_ = true;
  }
  cv_.notify_all();
}

#if 0

/* freertos simulator --------------------------------------------------------*/

FreertosSimulator::FreertosSimulator() {}

FreertosSimulator::FreertosSimulator(TickType_t tick_to_run)
    : tick_to_run_(tick_to_run), mode_(FreertosSimulatorMode::COUNT_DOWN) {}

void FreertosSimulator::start() {
  if (!task_code_) {
    throw std::runtime_error("task is not created before start");
  }
  task_thread_ = std::thread(task_code_);
  // wait some time for the thread creation to finish
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void FreertosSimulator::stop() {
  should_stop_ = true;
  task_thread_.join();
}

/* schedular management functions --------------------------------------------*/
TickType_t FreertosSimulator::xTaskGetTickCount() { return current_tick_; }

/* task management functions -------------------------------------------------*/
TaskHandle_t FreertosSimulator::xTaskCreateStatic(
    TaskFunction_t pxTaskCode, const char *const pcName,
    const uint32_t ulStackDepth, void *const pvParameters,
    UBaseType_t uxPriority, StackType_t *const puxStackBuffer,
    StaticTask_t *const pxTaskBuffer) {
  (void)ulStackDepth;
  (void)uxPriority;
  (void)puxStackBuffer;
  (void)pxTaskBuffer;

  task_code_ = std::bind(pxTaskCode, pvParameters);
  task_name_ = pcName;

  // just return the memory address of the task control block
  return (TaskHandle_t)pxTaskBuffer;
}

BaseType_t FreertosSimulator::xTaskDelayUntil(
    TickType_t *const pxPreviousWakeTime, const TickType_t xTimeIncrement) {
  // wait some time to prevent wierd instabilities
  std::this_thread::sleep_for(std::chrono::microseconds(100));

  if (mode_ == FreertosSimulatorMode::COUNT_DOWN &&
      current_tick_ >= tick_to_run_) {
    return MODULE_END;
  } else if (should_stop_.load()) {
    return MODULE_END;
  }

  *pxPreviousWakeTime = current_tick_;
  current_tick_ += xTimeIncrement;

  return pdTRUE;
}

/* task nofity functions -----------------------------------------------------*/
BaseType_t FreertosSimulator::xTaskNotify(TaskHandle_t xTaskToNotify,
                                          uint32_t ulValue,
                                          eNotifyAction eAction) {
  (void)xTaskToNotify;

  if (eAction != eSetBits) {
    throw std::runtime_error(
        "xTaskNotify does not spuuort notify action other than eSetBits");
  }

  {
    std::unique_lock<std::mutex> lock(task_notify_mutex_);
    task_notify_check_ = true;
    task_notify_value_ |= ulValue;
  }
  task_notify_cv_.notify_one();

  // wait some time for notify to take place on other thread
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  return pdPASS;
}

BaseType_t FreertosSimulator::xTaskNotifyFromISR(
    TaskHandle_t xTaskToNotify, uint32_t ulValue, eNotifyAction eAction,
    BaseType_t *pxHigherPriorityTaskWoken) {
  *pxHigherPriorityTaskWoken = pdFALSE;
  return xTaskNotify(xTaskToNotify, ulValue, eAction);
}

BaseType_t FreertosSimulator::xTaskNotifyWait(UBaseType_t ulBitsToClearOnEntry,
                                              uint32_t ulBitsToClearOnExit,
                                              uint32_t *pulNotificationValue,
                                              TickType_t xTicksToWait) {
  (void)xTicksToWait;

  std::unique_lock lock(task_notify_mutex_);
  task_notify_cv_.wait(lock, [this] { return task_notify_check_; });
  task_notify_check_ = false;

  task_notify_value_ &= ~ulBitsToClearOnEntry;
  *pulNotificationValue = task_notify_value_;
  task_notify_value_ &= ~ulBitsToClearOnExit;
  return pdPASS;
}

#endif

}  // namespace mock
