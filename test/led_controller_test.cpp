// stl include
#include <algorithm>
#include <future>

extern "C" {
// freertos include
#include "FreeRTOS.h"

// stm32 include
#include "stm32_module/stm32_hal.h"

// stm32_module include
#include "stm32_module/led_controller.h"
#include "stm32_module/module_common.h"
}

// gtest include
#include "gtest/gtest.h"

// mock include
#include "mock/freertos_mock.hpp"
#include "mock/hal_gpio_mock.hpp"
#include "mock/mock_common.hpp"

using ::testing::_;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Sequence;
using ::testing::Test;
using ::testing::WithArg;

/* test parameters -----------------------------------------------------------*/
// should be in (1, 7]
#define NUM_LED 5
#define REPEATED_TEST_TIMES 10

/* other variables -----------------------------------------------------------*/
// convenient array for numbering gpio ports
static GPIO_TypeDef* num_to_gpio_port[] = {GPIOA, GPIOB, GPIOC, GPIOD,
                                           GPIOE, GPIOF, GPIOG};

/* led controller initialization test ----------------------------------------*/
TEST(LedControllerInitTest, LedControllerCtor) {
  LedController led_controller;

  LedController_ctor(&led_controller);

  // really nothing to check
}

TEST(LedControllerInitTest, LedControllerInitLed) {
  LedController led_controller;
  struct led_cb led_cb;

  LedController_ctor(&led_controller);
  EXPECT_EQ(LedController_add_led(&led_controller, &led_cb, GPIOA, 0),
            ModuleOK);
}

/* led controller start test -------------------------------------------------*/
class LedControllerStartTest : public Test {
 protected:
  void SetUp() override { LedController_ctor(&led_controller_); }

  LedController led_controller_;

  struct led_cb led_cb_;

  HAL_GPIOMock gpio_mock_;

  FreertosMock freertos_mock_;
};

TEST_F(LedControllerStartTest, TurnOnOffBlinkWileNotStarted) {
  EXPECT_CALL(gpio_mock_, HAL_GPIO_WritePin).Times(0);
  EXPECT_CALL(freertos_mock_, xTaskCreateStatic).Times(0);

  EXPECT_EQ(LedController_turn_on(&led_controller_, 0), ModuleError);
  EXPECT_EQ(LedController_turn_off(&led_controller_, 0), ModuleError);
  EXPECT_EQ(LedController_blink(&led_controller_, 0, 0), ModuleError);
}

TEST_F(LedControllerStartTest, StartLedController) {
  EXPECT_CALL(freertos_mock_, xTaskCreateStatic)
      .WillOnce(
          WithArg<6>(Invoke([](StaticTask_t* t) { return (TaskHandle_t)t; })));

  LedController_add_led(&led_controller_, &led_cb_, num_to_gpio_port[0], 0);
  EXPECT_EQ(LedController_start(&led_controller_), ModuleOK);
}

TEST_F(LedControllerStartTest, AddLedWhileStarted) {
  EXPECT_CALL(freertos_mock_, xTaskCreateStatic)
      .WillOnce(
          WithArg<6>(Invoke([](StaticTask_t* t) { return (TaskHandle_t)t; })));

  LedController_add_led(&led_controller_, &led_cb_, num_to_gpio_port[0], 0);
  LedController_start(&led_controller_);
  EXPECT_EQ(
      LedController_add_led(&led_controller_, &led_cb_, num_to_gpio_port[1], 1),
      ModuleError);
}

/* led controller on off blink test ------------------------------------------*/
class LedControllerOnOffBlinkTest : public Test {
 protected:
  void SetUp() override {
    LedController_ctor(&led_controller_);
    for (int i = 0; i < NUM_LED; i++) {
      LedController_add_led(&led_controller_, &led_cb_[i], num_to_gpio_port[i],
                            i);
    }
    LedController_start(&led_controller_);
    // yield for led controller to run
    vPortYield();
  }

  void TearDown() override { Task_delete((Task*)&led_controller_); }

  LedController led_controller_;

  struct led_cb led_cb_[NUM_LED];

  HAL_GPIOMock gpio_mock_;
};

TEST_F(LedControllerOnOffBlinkTest, TurnOnLed) {
  for (int i = 0; i < NUM_LED; i++) {
    EXPECT_CALL(gpio_mock_,
                HAL_GPIO_WritePin(num_to_gpio_port[i], i, GPIO_PIN_SET))
        .Times(1);
  }

  for (int i = 0; i < NUM_LED; i++) {
    EXPECT_EQ(LedController_turn_on(&led_controller_, i), ModuleOK);
  }
}

TEST_F(LedControllerOnOffBlinkTest, TurnOnLedWhileOn) {
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_SET))
      .Times(1);

  EXPECT_EQ(LedController_turn_on(&led_controller_, 0), ModuleOK);
  EXPECT_EQ(LedController_turn_on(&led_controller_, 0), ModuleOK);
}

TEST_F(LedControllerOnOffBlinkTest, TurnOffLed) {
  {
    // force to expect ordered call
    InSequence seq;
    for (int i = 0; i < NUM_LED; i++) {
      EXPECT_CALL(gpio_mock_,
                  HAL_GPIO_WritePin(num_to_gpio_port[i], i, GPIO_PIN_SET))
          .Times(1);
    }
    for (int i = 0; i < NUM_LED; i++) {
      EXPECT_CALL(gpio_mock_,
                  HAL_GPIO_WritePin(num_to_gpio_port[i], i, GPIO_PIN_RESET))
          .Times(1);
    }
  }

  for (int i = 0; i < NUM_LED; i++) {
    EXPECT_EQ(LedController_turn_on(&led_controller_, i), ModuleOK);
  }

  for (int i = 0; i < NUM_LED; i++) {
    EXPECT_EQ(LedController_turn_off(&led_controller_, i), ModuleOK);
  }
}

TEST_F(LedControllerOnOffBlinkTest, TurnOffLedWhileOff) {
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_RESET))
      .Times(0);

  LedController_turn_off(&led_controller_, 0);
}

TEST_F(LedControllerOnOffBlinkTest, TurnOffLedWhileOffAndIsOnBefore) {
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_SET))
      .Times(1);
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_RESET))
      .Times(1);

  LedController_turn_on(&led_controller_, 0);
  LedController_turn_off(&led_controller_, 0);
  LedController_turn_off(&led_controller_, 0);
}

TEST_F(LedControllerOnOffBlinkTest, RepetedlyTurnOnAndOffLed) {
  {
    // force to expect ordered call
    InSequence seq;
    for (int i = 0; i < REPEATED_TEST_TIMES; i++) {
      EXPECT_CALL(gpio_mock_,
                  HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_SET))
          .Times(1);
      EXPECT_CALL(gpio_mock_,
                  HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_RESET))
          .Times(1);
    }
  }

  for (int i = 0; i < REPEATED_TEST_TIMES; i++) {
    LedController_turn_on(&led_controller_, 0);
    LedController_turn_off(&led_controller_, 0);
  }
}

TEST_F(LedControllerOnOffBlinkTest, BlinkLed) {
  std::promise<TickType_t> set_promise[NUM_LED], reset_promise[NUM_LED];
  std::future<TickType_t> set_future[NUM_LED], reset_future[NUM_LED];
  for (int i = 0; i < NUM_LED; i++) {
    set_future[i] = set_promise[i].get_future();
    reset_future[i] = reset_promise[i].get_future();
  }

  {
    // force to expect ordered call
    InSequence seq;
    for (int i = 0; i < NUM_LED; i++) {
      EXPECT_CALL(gpio_mock_,
                  HAL_GPIO_WritePin(num_to_gpio_port[i], i, GPIO_PIN_SET))
          .WillOnce([&, i] { set_promise[i].set_value(xTaskGetTickCount()); });
      EXPECT_CALL(gpio_mock_,
                  HAL_GPIO_WritePin(num_to_gpio_port[i], i, GPIO_PIN_RESET))
          .WillOnce(
              [&, i] { reset_promise[i].set_value(xTaskGetTickCount()); });
    }
  }
  for (int i = 0; i < NUM_LED; i++) {
    EXPECT_EQ(LedController_blink(&led_controller_, i, 100), ModuleOK);

    TickType_t period = reset_future[i].get() - set_future[i].get();
    EXPECT_LE(period, 100 + 10);
    EXPECT_GE(period, 100 - 10);
  }
}

TEST_F(LedControllerOnOffBlinkTest, RepeatedlyBlinkLed) {
  std::promise<TickType_t> set_promise[REPEATED_TEST_TIMES],
      reset_promise[REPEATED_TEST_TIMES];
  std::future<TickType_t> set_future[REPEATED_TEST_TIMES],
      reset_future[REPEATED_TEST_TIMES];
  for (int i = 0; i < REPEATED_TEST_TIMES; i++) {
    set_future[i] = set_promise[i].get_future();
    reset_future[i] = reset_promise[i].get_future();
  }

  {
    // force to expect ordered call
    InSequence seq;
    for (int i = 0; i < REPEATED_TEST_TIMES; i++) {
      EXPECT_CALL(gpio_mock_,
                  HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_SET))
          .WillOnce([&, i] { set_promise[i].set_value(xTaskGetTickCount()); });
      EXPECT_CALL(gpio_mock_,
                  HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_RESET))
          .WillOnce(
              [&, i] { reset_promise[i].set_value(xTaskGetTickCount()); });
    }
  }

  for (int i = 0; i < REPEATED_TEST_TIMES; i++) {
    TickType_t target_period = 10 * (i + 1);
    LedController_blink(&led_controller_, 0, target_period);

    TickType_t period = reset_future[i].get() - set_future[i].get();
    EXPECT_LE(period, target_period + 10);
    EXPECT_GE(period, target_period - 10);
  }
}

TEST_F(LedControllerOnOffBlinkTest, ConcurrentlyBlinkLed) {
  std::promise<TickType_t> set_promise[NUM_LED], reset_promise[NUM_LED];
  std::future<TickType_t> set_future[NUM_LED], reset_future[NUM_LED];
  for (int i = 0; i < NUM_LED; i++) {
    set_future[i] = set_promise[i].get_future();
    reset_future[i] = reset_promise[i].get_future();
  }

  // force to expect partially ordered call
  Sequence seq[NUM_LED];

  for (int i = 0; i < NUM_LED; i++) {
    EXPECT_CALL(gpio_mock_,
                HAL_GPIO_WritePin(num_to_gpio_port[i], i, GPIO_PIN_SET))
        .InSequence(seq[i])
        .WillOnce([&, i] { set_promise[i].set_value(xTaskGetTickCount()); });
    EXPECT_CALL(gpio_mock_,
                HAL_GPIO_WritePin(num_to_gpio_port[i], i, GPIO_PIN_RESET))
        .InSequence(seq[i])
        .WillOnce([&, i] { reset_promise[i].set_value(xTaskGetTickCount()); });
  }

  for (int i = 0; i < NUM_LED; i++) {
    LedController_blink(&led_controller_, i, 100);
  }

  TickType_t set_tick[NUM_LED], reset_tick[NUM_LED];
  for (int i = 0; i < NUM_LED; i++) {
    set_tick[i] = set_future[i].get();
    reset_tick[i] = reset_future[i].get();
  }

  // check if all led are turned and off at the same time
  auto set_tick_minmax = std::minmax_element(set_tick, set_tick + NUM_LED);
  auto reset_tick_minmax =
      std::minmax_element(reset_tick, reset_tick + NUM_LED);
  EXPECT_LE(*set_tick_minmax.second - *set_tick_minmax.first, 10);
  EXPECT_LE(*reset_tick_minmax.second - *reset_tick_minmax.first, 10);
}

TEST_F(LedControllerOnOffBlinkTest, BlinkLedWhileOn) {
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_SET))
      .Times(1);
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_RESET))
      .Times(0);

  LedController_turn_on(&led_controller_, 0);
  EXPECT_EQ(LedController_blink(&led_controller_, 0, 100), ModuleBusy);

  // wait some time to check if the led is ever turned off
  vTaskDelay(200);
}

TEST_F(LedControllerOnOffBlinkTest, TurnOnLedWhileBlinking) {
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_SET))
      .Times(1);
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_RESET))
      .Times(0);

  LedController_blink(&led_controller_, 0, 100);
  LedController_turn_on(&led_controller_, 0);

  // wait some time to check if the led is ever turned off
  vTaskDelay(200);
}

TEST_F(LedControllerOnOffBlinkTest, TurnOffLedWhileBlinking) {
  std::promise<TickType_t> set_promise, reset_promise;
  std::future<TickType_t> set_future = set_promise.get_future(),
                          reset_future = reset_promise.get_future();

  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_SET))
      .WillOnce([&] { set_promise.set_value(xTaskGetTickCount()); });
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_RESET))
      .WillOnce([&] { reset_promise.set_value(xTaskGetTickCount()); });

  LedController_blink(&led_controller_, 0, 100);
  LedController_turn_off(&led_controller_, 0);

  EXPECT_LE(reset_future.get() - set_future.get(), 10);
}

int main(int argc, char** argv) { return mock::run_freertos_test(&argc, argv); }
