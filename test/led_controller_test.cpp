// stl include
#include <algorithm>
#include <functional>
#include <future>

// freertos include
#include "FreeRTOS.h"

// stm32 include
#if defined(STM32G431xx)
#include "stm32g4xx_hal.h"
#elif defined(STM32H723xx)
#include "stm32h7xx_hal.h"
#endif

// gtest include
#include "gtest/gtest.h"

// stm32_module include
#include "stm32_module/led_controller.h"
#include "stm32_module/module_common.h"

// mock include
#include "mock/hal_gpio_mock.hpp"
#include "mock/mock_common.hpp"

// should be in (1, 10]
#define NUM_LED 5
#define REPEATED_TEST_TIMES 10

using ::testing::InSequence;
using ::testing::Sequence;
using ::testing::Test;

// convenient array for numbering gpio ports
static GPIO_TypeDef* num_to_gpio_port[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE,
                                           GPIOF, GPIOG, GPIOH, GPIOJ, GPIOK};

/* led controller initialization test ----------------------------------------*/
TEST(LedControllerInitTest, LedControllerCtor) {
  LedControlBlock led_control_block[NUM_LED];
  LedController led_controller;

  LedController_ctor(&led_controller, NUM_LED, led_control_block);

  EXPECT_EQ(led_controller.num_led_, NUM_LED);
  EXPECT_EQ(led_controller.led_control_block_, led_control_block);
}

TEST(LedControllerInitTest, LedControllerInitLed) {
  LedControlBlock led_control_block[NUM_LED];
  LedController led_controller;

  LedController_ctor(&led_controller, NUM_LED, led_control_block);

  LedController_init_led(&led_controller, 0, num_to_gpio_port[0], 0), MODULE_OK;
  EXPECT_EQ(led_controller.led_control_block_[0].led_port, GPIOA);
  EXPECT_EQ(led_controller.led_control_block_[0].led_pin, 0);
  EXPECT_EQ(led_controller.led_control_block_[0].state, LED_OFF);
}

/* led controller start test -------------------------------------------------*/
class LedControllerStartTest : public Test {
 protected:
  void SetUp() override {
    LedController_ctor(&led_controller_, NUM_LED, led_control_block_);
  }

  LedController led_controller_;

  LedControlBlock led_control_block_[NUM_LED];
};

TEST_F(LedControllerStartTest, TurnOnOffBlinkWileNotStarted) {
  HAL_GPIO_Mock gpio_mock;
  EXPECT_CALL(gpio_mock, HAL_GPIO_WritePin).Times(0);

  EXPECT_EQ(LedController_turn_on(&led_controller_, 0), MODULE_ERROR);
  EXPECT_EQ(LedController_turn_off(&led_controller_, 0), MODULE_ERROR);
  EXPECT_EQ(LedController_blink(&led_controller_, 0, 0), MODULE_ERROR);
}

TEST_F(LedControllerStartTest, PartiallyInitializedLed) {
  LedController_init_led(&led_controller_, 0, num_to_gpio_port[0], 0);
  EXPECT_EQ(LedController_start(&led_controller_), MODULE_ERROR);
}

TEST_F(LedControllerStartTest, FullyInitializedLed) {
  for (int i = 0; i < NUM_LED; i++) {
    LedController_init_led(&led_controller_, i, num_to_gpio_port[i], i);
  }
  EXPECT_EQ(LedController_start(&led_controller_), MODULE_OK);
  EXPECT_EQ(led_controller_.super_.state_, TASK_RUNNING);
}

#if 0

/* led controller on off test ------------------------------------------------*/
class LedControllerOnOffTest : public Test {
 protected:
  void SetUp() override {
    EXPECT_CALL(freertos_mock_, xTaskCreateStatic).Times(1);

    LedController_ctor(&led_controller_, NUM_LED, led_control_block_);
    for (int i = 0; i < NUM_LED; i++) {
      LedController_init_led(&led_controller_, i, num_to_gpio_port[i], i);
    }
    LedController_start(&led_controller_);
  }

  HAL_GPIO_Mock gpio_mock_;

  FreeRTOS_Mock freertos_mock_;

  LedController led_controller_;

  LedControlBlock led_control_block_[NUM_LED];
};

TEST_F(LedControllerOnOffTest, TurnOnLed) {
  for (int i = 0; i < NUM_LED; i++) {
    EXPECT_CALL(gpio_mock_,
                HAL_GPIO_WritePin(num_to_gpio_port[i], i, GPIO_PIN_SET))
        .Times(1);
  }

  for (int i = 0; i < NUM_LED; i++) {
    EXPECT_EQ(LedController_turn_on(&led_controller_, i), MODULE_OK);
  }
}

TEST_F(LedControllerOnOffTest, TurnOnLedWhileOn) {
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_SET))
      .Times(1);

  EXPECT_EQ(LedController_turn_on(&led_controller_, 0), MODULE_OK);
  EXPECT_EQ(LedController_turn_on(&led_controller_, 0), MODULE_OK);
}

TEST_F(LedControllerOnOffTest, TurnOffLed) {
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
    EXPECT_EQ(LedController_turn_on(&led_controller_, i), MODULE_OK);
  }
  for (int i = 0; i < NUM_LED; i++) {
    EXPECT_EQ(LedController_turn_off(&led_controller_, i), MODULE_OK);
  }
}

TEST_F(LedControllerOnOffTest, TurnOffLedWhileOff) {
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_RESET))
      .Times(0);

  LedController_turn_off(&led_controller_, 0);
}

TEST_F(LedControllerOnOffTest, TurnOffLedWhileOffAndIsOnBefore) {
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

TEST_F(LedControllerOnOffTest, RepetedlyTurnOnAndOffLed) {
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

/* led controller blink test -------------------------------------------------*/
class LedControllerBlinkTest : public Test {
 protected:
  void SetUp() override {
    SIMULATE_FREERTOS_CALL_ONCE(freertos_mock_, freertos_simulator_,
                                xTaskCreateStatic);
    SIMULATE_FREERTOS_CALL(freertos_mock_, freertos_simulator_,
                           xTaskGetTickCount);
    SIMULATE_FREERTOS_CALL(freertos_mock_, freertos_simulator_,
                           xTaskDelayUntil);

    LedController_ctor(&led_controller_, NUM_LED, led_control_block_);
    for (int i = 0; i < NUM_LED; i++) {
      LedController_init_led(&led_controller_, i, num_to_gpio_port[i], i);
    }
    LedController_start(&led_controller_);

    freertos_simulator_.start();
  }

  void TearDown() override { freertos_simulator_.stop(); }

  HAL_GPIO_Mock gpio_mock_;

  FreeRTOS_Mock freertos_mock_;

  FreertosSimulator freertos_simulator_;

  LedController led_controller_;

  LedControlBlock led_control_block_[NUM_LED];
};

TEST_F(LedControllerBlinkTest, BlinkLed) {
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
          .WillOnce([&, i] {
            set_promise[i].set_value(freertos_simulator_.xTaskGetTickCount());
          });
      EXPECT_CALL(gpio_mock_,
                  HAL_GPIO_WritePin(num_to_gpio_port[i], i, GPIO_PIN_RESET))
          .WillOnce([&, i] {
            reset_promise[i].set_value(freertos_simulator_.xTaskGetTickCount());
          });
    }
  }
  for (int i = 0; i < NUM_LED; i++) {
    EXPECT_EQ(LedController_blink(&led_controller_, i, 100), MODULE_OK);

    TickType_t period = reset_future[i].get() - set_future[i].get();
    EXPECT_LE(period, 100 + 10);
    EXPECT_GE(period, 100 - 10);
  }
}

TEST_F(LedControllerBlinkTest, RepeatedlyBlinkLed) {
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
          .WillOnce([&, i] {
            set_promise[i].set_value(freertos_simulator_.xTaskGetTickCount());
          });
      EXPECT_CALL(gpio_mock_,
                  HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_RESET))
          .WillOnce([&, i] {
            reset_promise[i].set_value(freertos_simulator_.xTaskGetTickCount());
          });
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

TEST_F(LedControllerBlinkTest, ConcurrentlyBlinkLed) {
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
        .WillOnce([&, i] {
          set_promise[i].set_value(freertos_simulator_.xTaskGetTickCount());
        });
    EXPECT_CALL(gpio_mock_,
                HAL_GPIO_WritePin(num_to_gpio_port[i], i, GPIO_PIN_RESET))
        .InSequence(seq[i])
        .WillOnce([&, i] {
          reset_promise[i].set_value(freertos_simulator_.xTaskGetTickCount());
        });
  }

  for (int i = 0; i < NUM_LED; i++) {
    LedController_blink(&led_controller_, i, 100);
  }

  TickType_t set_tick[NUM_LED], reset_tick[NUM_LED];
  for (int i = 0; i < NUM_LED; i++) {
    set_tick[i] = set_future[i].get();
    reset_tick[i] = reset_future[i].get();
  }

  auto set_tick_minmax = std::minmax_element(set_tick, set_tick + NUM_LED);
  auto reset_tick_minmax =
      std::minmax_element(reset_tick, reset_tick + NUM_LED);
  EXPECT_LE(*set_tick_minmax.second - *set_tick_minmax.first, 10);
  EXPECT_LE(*reset_tick_minmax.second - *reset_tick_minmax.first, 10);
}

TEST_F(LedControllerBlinkTest, BlinkLedWhileOn) {
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_SET))
      .Times(1);
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_RESET))
      .Times(0);

  LedController_turn_on(&led_controller_, 0);
  EXPECT_EQ(LedController_blink(&led_controller_, 0, 100), MODULE_BUSY);

  // wait some time to check if the led is ever turned off
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

TEST_F(LedControllerBlinkTest, TurnOnLedWhileBlinking) {
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_SET))
      .Times(1);
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_RESET))
      .Times(0);

  LedController_blink(&led_controller_, 0, 100);
  LedController_turn_on(&led_controller_, 0);

  // wait some time to check if the led is ever turned off
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

TEST_F(LedControllerBlinkTest, TurnOffLedWhileBlinking) {
  std::promise<TickType_t> set_promise, reset_promise;
  std::future<TickType_t> set_future = set_promise.get_future(),
                          reset_future = reset_promise.get_future();

  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_SET))
      .WillOnce([&] {
        set_promise.set_value(freertos_simulator_.xTaskGetTickCount());
      });
  EXPECT_CALL(gpio_mock_,
              HAL_GPIO_WritePin(num_to_gpio_port[0], 0, GPIO_PIN_RESET))
      .WillOnce([&] {
        reset_promise.set_value(freertos_simulator_.xTaskGetTickCount());
      });

  LedController_blink(&led_controller_, 0, 100);
  LedController_turn_off(&led_controller_, 0);

  EXPECT_LE(reset_future.get() - set_future.get(), 10);
}

#endif