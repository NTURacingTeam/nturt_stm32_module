// stl incldue
#include <cstdint>

extern "C" {
// freertos include
#include "FreeRTOS.h"

// stm32_module include
#include "stm32_module/module_common.h"
}

// gtest include
#include "gtest/gtest.h"

// mock include
#include "mock/freertos_mock.hpp"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Test;
using ::testing::WithArg;

TEST(TaskInitTest, TaskCtor) {
  Task task;
  // fake task_code function pointer
  TaskFunction_t task_code = (TaskFunction_t)0x12345678;

  Task_ctor(&task, task_code);
  EXPECT_EQ(task.state_, TaskReset);
  EXPECT_EQ(task.task_code_, task_code);
}

class TaskTest : public Test {
 protected:
  void SetUp() override { Task_ctor(&task_, task_code_); }

  Task task_;

  // fake task_code function pointer
  TaskFunction_t task_code_ = (TaskFunction_t)0x12345678;

  const char* task_name_ = "test_task";

  UBaseType_t task_priority = 1;

  uint32_t stack_depth_ = 10;

  StackType_t stack_buffer_[10];

  FreertosMock freertos_mock_;
};

TEST_F(TaskTest, StartFreertosTask) {
  EXPECT_CALL(freertos_mock_,
              xTaskCreateStatic(task_code_, task_name_, stack_depth_, &task_,
                                task_priority, stack_buffer_, _))
      .WillOnce(
          WithArg<6>(Invoke([](StaticTask_t* t) { return (TaskHandle_t)t; })));

  Task_create_freertos_task(&task_, task_name_, task_priority, stack_buffer_,
                            stack_depth_);
}
