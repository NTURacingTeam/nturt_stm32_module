// stl include
#include <functional>

// freertos include
#include "FreeRTOS.h"

// gtest include
#include "gtest/gtest.h"

// stm32_module include
#include "stm32_module/error_handler.h"
#include "stm32_module/module_common.h"

// mock include
#include "mock/mock_common.hpp"

using ::testing::Return;
using ::testing::Test;

/* error handler initialization test -----------------------------------------*/
TEST(ErrorHandlerInitTest, ErrorHandlerCtor) {
  ErrorHandler error_handler;

  ErrorHandler_ctor(&error_handler);

  EXPECT_EQ(error_handler.error_code_, 0);
}

/* error handler start test --------------------------------------------------*/
class ErrorHandlerStartTest : public Test {
 protected:
  void SetUp() override { ErrorHandler_ctor(&error_handler_); }

  ErrorHandler error_handler_;
};

TEST_F(ErrorHandlerStartTest, AccessErrorWhileNotStarted) {
  EXPECT_EQ(
      ErrorHandler_write_error(&error_handler_, ERROR_CODE_CAN_TRANSMIT_ERROR,
                               ERROR_OPTION_SET),
      MODULE_ERROR);
}

TEST_F(ErrorHandlerStartTest, ErrorHandlerStart) {
  EXPECT_EQ(ErrorHandler_start(&error_handler_), MODULE_OK);
}

#if 0

/* error handler access error test -------------------------------------------*/
class ErrorHandlerAccessErrorTest : public Test {
 protected:
  void SetUp() override {
    EXPECT_CALL(freertos_mock_, xPortIsInsideInterrupt)
        .WillRepeatedly(Return(pdFALSE));
    SIMULATE_FREERTOS_CALL_ONCE(freertos_mock_, freertos_simulator_,
                                xTaskCreateStatic);
    SIMULATE_FREERTOS_CALL(freertos_mock_, freertos_simulator_,
                           xTaskGetTickCount);
    SIMULATE_FREERTOS_CALL(freertos_mock_, freertos_simulator_,
                           xTaskDelayUntil);
    SIMULATE_FREERTOS_CALL(freertos_mock_, freertos_simulator_, xTaskNotify);
    SIMULATE_FREERTOS_CALL(freertos_mock_, freertos_simulator_,
                           xTaskNotifyWait);

    ErrorHandler_ctor(&error_handler_);
    ErrorHandler_start(&error_handler_);

    freertos_simulator_.start();
  }

  void TearDown() override { freertos_simulator_.stop(); }

  FreeRTOS_Mock freertos_mock_;

  FreertosSimulator freertos_simulator_;

  ErrorHandler error_handler_;
};

TEST_F(ErrorHandlerAccessErrorTest, ErrorHandlerWriteError) {
  EXPECT_EQ(ErrorHandler_write_error(&error_handler_,
                                     ERROR_CODE_CAN_TRANSMIT_ERROR |
                                         ERROR_CODE_CAN_RECEIVE_TIMEOUT_ERROR,
                                     ERROR_OPTION_SET),
            MODULE_OK);
  EXPECT_EQ(
      error_handler_.error_code_,
      ERROR_CODE_CAN_TRANSMIT_ERROR | ERROR_CODE_CAN_RECEIVE_TIMEOUT_ERROR);

  EXPECT_EQ(
      ErrorHandler_write_error(&error_handler_, ERROR_CODE_CAN_TRANSMIT_ERROR,
                               ERROR_OPTION_CLEAR),
      MODULE_OK);

  EXPECT_EQ(error_handler_.error_code_, ERROR_CODE_CAN_RECEIVE_TIMEOUT_ERROR);
}

TEST_F(ErrorHandlerAccessErrorTest, ErrorHandlerGetErrorTest) {
  ErrorHandler_write_error(
      &error_handler_,
      ERROR_CODE_CAN_TRANSMIT_ERROR | ERROR_CODE_CAN_RECEIVE_TIMEOUT_ERROR,
      ERROR_OPTION_SET);
  EXPECT_EQ(
      ErrorHandler_get_error(&error_handler_),
      ERROR_CODE_CAN_TRANSMIT_ERROR | ERROR_CODE_CAN_RECEIVE_TIMEOUT_ERROR);

  ErrorHandler_write_error(&error_handler_, ERROR_CODE_CAN_TRANSMIT_ERROR,
                           ERROR_OPTION_CLEAR);
  EXPECT_EQ(ErrorHandler_get_error(&error_handler_),
            ERROR_CODE_CAN_RECEIVE_TIMEOUT_ERROR);
}

#endif
