#ifndef MOCK_COMMON_HPP
#define MOCK_COMMON_HPP

// stl include
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>

extern "C" {
// freertos include
#include "FreeRTOS.h"
#include "task.h"

// stm32_module include
#include "stm32_module/module_common.h"
}

// gtest include
#include "cmock/cmock.h"
#include "gtest/gtest.h"

/* type ----------------------------------------------------------------------*/
enum class FreertosSimulatorMode {
  Countdown,
  Continuous,
};

/* mock ----------------------------------------------------------------------*/
/// @brief Class for mocking common function using google test framework.
class CommonMock : public CMockMocker<CommonMock> {
 public:
  CommonMock();

  ~CommonMock();

  CMOCK_MOCK_METHOD(void, __module_assert_fail,
                    (const char *, const char *, unsigned int, const char *));
};

namespace testing {

template <typename T>
class CArrayMatcher : public MatcherInterface<T> {
 public:
  explicit CArrayMatcher(T expected_array, int array_length)
      : expected_array_(expected_array), array_length_(array_length) {}

  bool MatchAndExplain(T array,
                       MatchResultListener * /*listener*/) const override {
    return std::memcmp(array, expected_array_,
                       sizeof(expected_array_[0]) * array_length_) == 0;
  }

  void DescribeTo(::std::ostream *os) const override {
    if (std::is_same<T, const char *>::value ||
        std::is_same<T, char *>::value) {
      *os << "is equal to c string \"" << expected_array_ << "\"";
    } else {
      *os << "is equal to {";
      for (int i = 0; i < array_length_; ++i) {
        *os << std::to_string(expected_array_[i]);
        if (i != array_length_ - 1) {
          *os << ", ";
        }
      }
      *os << "}";
    }
  }

  void DescribeNegationTo(::std::ostream *os) const override {
    if (std::is_same<T, const char *>::value ||
        std::is_same<T, char *>::value) {
      *os << "isn't equal to c string \"" << expected_array_ << "\"";
    } else {
      *os << "isn't equal to {";
      for (int i = 0; i < array_length_; ++i) {
        *os << std::to_string(expected_array_[i]);
        if (i != array_length_ - 1) {
          *os << ", ";
        }
      }
      *os << "}";
    }
  }

 private:
  T expected_array_;
  int array_length_;
};

/**
 * @brief Matcher for c-style array.
 *
 * @tparam T Type of array.
 * @param expected_array Expected array.
 * @param array_length The length of array, which is the number of elements in
 * that array.
 *
 * @note If the array's element type is not a primitive type, it should have a
 * overloaded std::to_string() function, or it will not compile
 */
template <typename T>
Matcher<T> ArrayWithSize(T expected_array, int array_length) {
  return Matcher<T>(new CArrayMatcher(expected_array, array_length));
}

}  // namespace testing

namespace mock {

/**
 * @brief Convenient function for running freertos based tests. Initializes
 * google test and runs the tests.
 *
 * @param argc Pointer to argc from main.
 * @param argv Pointer to argv from main.
 * @return int Return code from running gtest RUN_ALL_TESTS().
 */
int run_freertos_test(int *argc, char **argv);

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
  FreertosSimulatorMode mode_ = FreertosSimulatorMode::Continuous;

  TickType_t current_tick_ = 0;

  TickType_t tick_to_run_ = 0;

  std::atomic_bool should_stop_ = false;
};

#endif

}  // namespace mock

#endif  // MOCK_COMMON_HPP
