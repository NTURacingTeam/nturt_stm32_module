// stl include

// stm32 include
#if defined(STM32G431xx)
#include "stm32g4xx_hal.h"
#elif defined(STM32H723xx)
#include "stm32h7xx_hal.h"
#endif

// gtest include
#include "gtest/gtest.h"

// stm32_module include
#include "stm32_module/can_transceiver.h"
#include "stm32_module/module_common.h"

// mock include
#include "mock/hal_can_mock.hpp"
#include "mock/mock_common.hpp"

using ::testing::Test;

/* can transceiver initialization test ---------------------------------------*/
TEST(CanTransceiverInitTest, CanTransceiverCtor) {
  CanTransceiver can_transceiver;
  FDCAN_HandleTypeDef can_handle;

  CanTransceiver_ctor(&can_transceiver, &can_handle);

  EXPECT_EQ(can_transceiver.can_handle_, &can_handle);
}

class CanTransceiverStartTest : public Test {
 protected:
  void SetUp() override {
    CanTransceiver_ctor(&can_transceiver_, &fdcan_handle_);
  }

  CanTransceiver can_transceiver_;

  FDCAN_HandleTypeDef fdcan_handle_;
};

TEST_F(CanTransceiverStartTest, CanTransceiverStart) {
  EXPECT_EQ(CanTransceiver_start(&can_transceiver_), MODULE_OK);
  EXPECT_EQ(can_transceiver_.super_.state_, TASK_RUNNING);
}
