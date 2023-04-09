extern "C" {
// freertos include
#include "FreeRTOS.h"

// stm32_module include
#include "stm32_module/error_handler.h"
#include "stm32_module/module_common.h"
}

// gtest include
#include "gtest/gtest.h"

// mock include
#include "mock/freertos_mock.hpp"
#include "mock/mock_common.hpp"

using ::testing::Invoke;
using ::testing::Return;
using ::testing::Test;
using ::testing::WithArg;

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

  FreertosMock freertos_mock_;
};

TEST_F(ErrorHandlerStartTest, AccessErrorWhileNotStarted) {
  EXPECT_EQ(
      ErrorHandler_write_error(&error_handler_, ERROR_CODE_CAN_TRANSMIT_ERROR,
                               ERROR_OPTION_SET),
      ModuleError);
}

TEST_F(ErrorHandlerStartTest, ErrorHandlerStart) {
  EXPECT_CALL(freertos_mock_, xTaskCreateStatic)
      .WillOnce(
          WithArg<6>(Invoke([](StaticTask_t* t) { return (TaskHandle_t)t; })));

  EXPECT_EQ(ErrorHandler_start(&error_handler_), ModuleOK);
  EXPECT_EQ(error_handler_.super_.state_, TaskRunning);
}

/* error handler access error test -------------------------------------------*/
class ErrorHandlerAccessErrorTest : public Test {
 protected:
  void SetUp() override {
    ErrorHandler_ctor(&error_handler_);
    ErrorHandler_start(&error_handler_);
  }

  void TearDown() override { Task_delete((Task*)&error_handler_); }

  ErrorHandler error_handler_;
};

TEST_F(ErrorHandlerAccessErrorTest, ErrorHandlerWriteError) {
  EXPECT_EQ(ErrorHandler_write_error(&error_handler_,
                                     ERROR_CODE_CAN_TRANSMIT_ERROR |
                                         ERROR_CODE_CAN_RECEIVE_TIMEOUT_ERROR,
                                     ERROR_OPTION_SET),
            ModuleOK);
  EXPECT_EQ(
      error_handler_.error_code_,
      ERROR_CODE_CAN_TRANSMIT_ERROR | ERROR_CODE_CAN_RECEIVE_TIMEOUT_ERROR);

  EXPECT_EQ(
      ErrorHandler_write_error(&error_handler_, ERROR_CODE_CAN_TRANSMIT_ERROR,
                               ERROR_OPTION_CLEAR),
      ModuleOK);

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

int main(int argc, char** argv) { return mock::run_freertos_test(&argc, argv); }
