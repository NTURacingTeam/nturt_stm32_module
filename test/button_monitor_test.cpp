// stl include
#include <future>

extern "C" {
// stm32 include
#include "stm32_module/stm32_hal.h"

// stm32_module include
#include "stm32_module/stm32_module.h"
}

// gtest include
#include "cmock/cmock.h"
#include "gtest/gtest.h"

// mock include
#include "mock/mock.hpp"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Test;
using ::testing::WithArg;

/* test parameters -----------------------------------------------------------*/
// should be in (1, 7]
#define NUM_BUTTON 5
#define REPEATED_TEST_TIMES 10

/* other variables -----------------------------------------------------------*/
// convenient array for numbering gpio ports
static GPIO_TypeDef* num_to_gpio_port[] = {GPIOA, GPIOB, GPIOC, GPIOD,
                                           GPIOE, GPIOF, GPIOG};

/* button monitor initialization test ----------------------------------------*/
TEST(ButtonMonitorInitTest, ButtonMonitorCtor) {
  ButtonMonitor button_monitor;

  ButtonMonitor_ctor(&button_monitor);

  // really nothing to check
}

TEST(ButtonMonitorInitTest, ButtonMonitorAddButton) {
  HAL_GPIOMock gpio_mock_;
  EXPECT_CALL(gpio_mock_, HAL_GPIO_ReadPin).WillOnce(Return(GPIO_PIN_RESET));

  ButtonMonitor button_monitor;
  struct button_cb button_cb;

  ButtonMonitor_ctor(&button_monitor);
  EXPECT_EQ(ButtonMonitor_add_button(&button_monitor, &button_cb,
                                     num_to_gpio_port[0], 0),
            ModuleOK);
}

/* button monitor start test -------------------------------------------------*/
class ButtonMonitorStartTest : public Test {
 protected:
  void SetUp() override { ButtonMonitor_ctor(&button_monitor_); }

  ButtonMonitor button_monitor_;

  struct button_cb button_cb_;

  HAL_GPIOMock gpio_mock_;

  FreertosMock freertos_mock_;
};

TEST_F(ButtonMonitorStartTest, ReadStateWileNotStarted) {
  EXPECT_CALL(gpio_mock_, HAL_GPIO_ReadPin).WillOnce(Return(GPIO_PIN_RESET));
  EXPECT_CALL(freertos_mock_, xTaskCreateStatic).Times(0);

  GPIO_PinState state;

  ButtonMonitor_add_button(&button_monitor_, &button_cb_, num_to_gpio_port[0],
                           0);
  EXPECT_EQ(ButtonMonitor_read_state(&button_monitor_, 0, &state), ModuleError);
}

TEST_F(ButtonMonitorStartTest, StartButtonMonitor) {
  EXPECT_CALL(gpio_mock_, HAL_GPIO_ReadPin).WillOnce(Return(GPIO_PIN_RESET));
  EXPECT_CALL(freertos_mock_, xTaskCreateStatic)
      .WillOnce(
          WithArg<6>(Invoke([](StaticTask_t* t) { return (TaskHandle_t)t; })));

  ButtonMonitor_add_button(&button_monitor_, &button_cb_, num_to_gpio_port[0],
                           0);
  EXPECT_EQ(ButtonMonitor_start(&button_monitor_), ModuleOK);
}

TEST_F(ButtonMonitorStartTest, AddButtonWhileStarted) {
  EXPECT_CALL(gpio_mock_, HAL_GPIO_ReadPin).WillOnce(Return(GPIO_PIN_RESET));
  EXPECT_CALL(freertos_mock_, xTaskCreateStatic)
      .WillOnce(
          WithArg<6>(Invoke([](StaticTask_t* t) { return (TaskHandle_t)t; })));

  ButtonMonitor_add_button(&button_monitor_, &button_cb_, num_to_gpio_port[0],
                           0);
  ButtonMonitor_start(&button_monitor_);
  EXPECT_EQ(ButtonMonitor_add_button(&button_monitor_, &button_cb_,
                                     num_to_gpio_port[1], 1),
            ModuleError);
}

/* button monitor read state test --------------------------------------------*/
class ButtonMonitorReadStateTest : public Test {
 protected:
  void SetUp() override {
    EXPECT_CALL(gpio_mock_, HAL_GPIO_ReadPin)
        .WillRepeatedly(Return(GPIO_PIN_RESET));
    ButtonMonitor_ctor(&button_monitor_);
    for (int i = 0; i < NUM_BUTTON; i++) {
      ButtonMonitor_add_button(&button_monitor_, &button_cb_[i],
                               num_to_gpio_port[i], i);
    }
    ButtonMonitor_start(&button_monitor_);
    // yield for button monitor to run
    vPortYield();
  }

  void TearDown() override { Task_delete((Task*)&button_monitor_); }

  ButtonMonitor button_monitor_;

  struct button_cb button_cb_[NUM_BUTTON];

  HAL_GPIOMock gpio_mock_;

  int call_count_[NUM_BUTTON] = {0};

  mock::Notification notification_[NUM_BUTTON];

  GPIO_PinState state_;
};

TEST_F(ButtonMonitorReadStateTest, ReadStateWithInvalidButtonNumber) {
  EXPECT_EQ(ButtonMonitor_read_state(&button_monitor_, NUM_BUTTON, &state_),
            ModuleError);
}

TEST_F(ButtonMonitorReadStateTest, CleanTransition) {
  for (int i = 0; i < NUM_BUTTON; i++) {
    EXPECT_CALL(gpio_mock_, HAL_GPIO_ReadPin(num_to_gpio_port[i], i))
        .WillRepeatedly(Invoke([&, i]() {
          if (++call_count_[i] == BUTTON_MONITOR_DEBOUNCE_TIMES + 1) {
            notification_[i].notify_one();
          }
          return GPIO_PIN_SET;
        }));
  }

  for (int i = 0; i < NUM_BUTTON; i++) {
    notification_[i].wait();
    EXPECT_EQ(ButtonMonitor_read_state(&button_monitor_, i, &state_), ModuleOK);
    EXPECT_EQ(state_, GPIO_PIN_SET);
  }
}

TEST_F(ButtonMonitorReadStateTest, BouncyTransition) {
  for (int i = 0; i < NUM_BUTTON; i++) {
    EXPECT_CALL(gpio_mock_, HAL_GPIO_ReadPin(num_to_gpio_port[i], i))
        .WillRepeatedly(Invoke([&, i]() {
          if (++call_count_[i] % (BUTTON_MONITOR_DEBOUNCE_TIMES + 1) == 0) {
            notification_[i].notify_one();
          }
          return (call_count_[i] % BUTTON_MONITOR_DEBOUNCE_TIMES == 0)
                     ? GPIO_PIN_SET
                     : GPIO_PIN_RESET;
        }));
  }

  for (int i = 0; i < REPEATED_TEST_TIMES; i++) {
    for (int j = 0; j < NUM_BUTTON; j++) {
      notification_[j].wait();
      ButtonMonitor_read_state(&button_monitor_, j, &state_);
      EXPECT_EQ(state_, GPIO_PIN_RESET);
    }
  }
}

/* button monitor callback test ----------------------------------------------*/
class ButtonMonitorCallbackTest : public Test {
 protected:
  void SetUp() override {
    EXPECT_CALL(gpio_mock_, HAL_GPIO_ReadPin)
        .WillRepeatedly(Return(GPIO_PIN_RESET));
    ButtonMonitor_ctor(&button_monitor_);
    for (int i = 0; i < NUM_BUTTON; i++) {
      ButtonMonitor_add_button(&button_monitor_, &button_cb_[i],
                               num_to_gpio_port[i], i);
      ButtonMonitor_register_callback(&button_monitor_, i, button_callback,
                                      nullptr);
    }
    ButtonMonitor_start(&button_monitor_);
    // yield for button monitor to run
    vPortYield();
  }

  void TearDown() override { Task_delete((Task*)&button_monitor_); }

  ButtonMonitor button_monitor_;

  struct button_cb button_cb_[NUM_BUTTON];

  HAL_GPIOMock gpio_mock_;

  CallbackMock callback_mock_;
};

TEST_F(ButtonMonitorCallbackTest, SetCallback) {
  EXPECT_CALL(gpio_mock_, HAL_GPIO_ReadPin)
      .WillRepeatedly(Return(GPIO_PIN_SET));
  EXPECT_CALL(callback_mock_, button_callback(_, GPIO_PIN_SET))
      .Times(NUM_BUTTON);

  // wait some time for button monitor to run
  vTaskDelay(10 * BUTTON_MONITOR_TASK_PERIOD);
}

TEST_F(ButtonMonitorCallbackTest, ResetCallback) {
  EXPECT_CALL(gpio_mock_, HAL_GPIO_ReadPin)
      .WillRepeatedly(Return(GPIO_PIN_SET));
  EXPECT_CALL(callback_mock_, button_callback(_, GPIO_PIN_SET))
      .Times(NUM_BUTTON);

  // wait some time for button monitor to run
  vTaskDelay(10 * BUTTON_MONITOR_TASK_PERIOD);

  EXPECT_CALL(gpio_mock_, HAL_GPIO_ReadPin)
      .WillRepeatedly(Return(GPIO_PIN_RESET));
  EXPECT_CALL(callback_mock_, button_callback(_, GPIO_PIN_RESET))
      .Times(NUM_BUTTON);

  // wait some time for button monitor to run
  vTaskDelay(10 * BUTTON_MONITOR_TASK_PERIOD);
}

TEST_F(ButtonMonitorCallbackTest, RepeatlyCallback) {
  for (int i = 0; i < REPEATED_TEST_TIMES; i++) {
    EXPECT_CALL(gpio_mock_, HAL_GPIO_ReadPin)
        .WillRepeatedly(Return(GPIO_PIN_SET));
    EXPECT_CALL(callback_mock_, button_callback(_, GPIO_PIN_SET))
        .Times(NUM_BUTTON);

    // wait some time for button monitor to run
    vTaskDelay(10 * BUTTON_MONITOR_TASK_PERIOD);

    EXPECT_CALL(gpio_mock_, HAL_GPIO_ReadPin)
        .WillRepeatedly(Return(GPIO_PIN_RESET));
    EXPECT_CALL(callback_mock_, button_callback(_, GPIO_PIN_RESET))
        .Times(NUM_BUTTON);

    // wait some time for button monitor to run
    vTaskDelay(10 * BUTTON_MONITOR_TASK_PERIOD);
  }
}

int main(int argc, char** argv) { return mock::run_freertos_test(&argc, argv); }
