// stl include
#include <cstdint>

extern "C" {
// freertos include
#include "FreeRTOS.h"

// stm32_module include
#include "stm32_module/stm32_module.h"
}

// gtest include
#include "gtest/gtest.h"

// mock include
#include "mock/mock.hpp"

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

TEST_F(ErrorHandlerStartTest, WriteGetErrorCodeWhileNotStarted) {
  uint32_t error_code = 0x12345678;
  EXPECT_EQ(ErrorHandler_write_error(&error_handler_, error_code, ERROR_SET),
            ModuleError);
  EXPECT_EQ(ErrorHandler_get_error(&error_handler_, &error_code), ModuleError);
  EXPECT_EQ(error_code, 0x12345678);
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

  uint32_t error_code_;
};

TEST_F(ErrorHandlerAccessErrorTest, ErrorHandlerWriteError) {
  EXPECT_EQ(ErrorHandler_write_error(
                &error_handler_, ERROR_CODE_CAN_TX | ERROR_CODE_CAN_RX_CRITICAL,
                ERROR_SET),
            ModuleOK);
  EXPECT_EQ(ErrorHandler_get_error(&error_handler_, &error_code_), ModuleOK);
  EXPECT_EQ(error_code_, ERROR_CODE_CAN_TX | ERROR_CODE_CAN_RX_CRITICAL);

  ErrorHandler_write_error(&error_handler_, ERROR_CODE_CAN_TX, ERROR_CLEAR);
  ErrorHandler_get_error(&error_handler_, &error_code_);
  EXPECT_EQ(error_code_, ERROR_CODE_CAN_RX_CRITICAL);
}

int main(int argc, char** argv) { return mock::run_freertos_test(&argc, argv); }
