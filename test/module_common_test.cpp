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

/* test parameters -----------------------------------------------------------*/
#define NUM_LIST_ITEM 10
#define REPEATED_TEST_TIMES 10

/* list initialization test --------------------------------------------------*/
TEST(ListInitTest, ListCtor) {
  List list;

  List_ctor(&list);

  // really nothing to test
}

/* list not item test --------------------------------------------------------*/
class ListNoItemTest : public Test {
 protected:
  void SetUp() override { List_ctor(&list_); }

  List list_;
};

TEST_F(ListNoItemTest, Size) { EXPECT_EQ(List_size(&list_), 0); }

TEST_F(ListNoItemTest, At) { EXPECT_EQ(List_at(&list_, 0), nullptr); }

/* list push back test -------------------------------------------------------*/
class ListPushBackTest : public Test {
 protected:
  void SetUp() override {
    List_ctor(&list_);
    for (int i = 0; i < NUM_LIST_ITEM; i++) {
      List_push_back(&list_, &list_cb_[i], (void*)i);
    }
  }

  List list_;

  struct list_cb list_cb_[NUM_LIST_ITEM];
};

TEST_F(ListPushBackTest, Size) { EXPECT_EQ(List_size(&list_), NUM_LIST_ITEM); }

TEST_F(ListPushBackTest, At) {
  for (int i = 0; i < NUM_LIST_ITEM; i++) {
    EXPECT_EQ(List_at(&list_, i), (void*)i);
  }
}

TEST_F(ListPushBackTest, OutOfRangeAt) {
  for (int i = 0; i < REPEATED_TEST_TIMES; i++) {
    EXPECT_EQ(List_at(&list_, NUM_LIST_ITEM + i), nullptr);
  }
}

/* list iterator initialization test -----------------------------------------*/
TEST(ListItorTest, ListItorCtor) {
  List list;
  ListIter list_iter;

  ListIter_ctor(&list_iter, &list);

  EXPECT_EQ(list_iter.index_, list.head_);
}

/* List iterator test --------------------------------------------------------*/
class ListIterTest : public Test {
 protected:
  void SetUp() override {
    List_ctor(&list_);
    for (int i = 0; i < NUM_LIST_ITEM; i++) {
      List_push_back(&list_, &list_cb_[i], (void*)i);
    }

    ListIter_ctor(&list_iter_, &list_);
  }

  List list_;

  struct list_cb list_cb_[NUM_LIST_ITEM];

  ListIter list_iter_;
};

TEST_F(ListIterTest, Next) {
  for (int i = 0; i < NUM_LIST_ITEM; i++) {
    EXPECT_EQ(ListIter_next(&list_iter_), (void*)i);
  }
}

TEST_F(ListIterTest, OutOfRangeNext) {
  for (int i = 0; i < NUM_LIST_ITEM; i++) {
    EXPECT_EQ(ListIter_next(&list_iter_), (void*)i);
  }

  for (int i = 0; i < REPEATED_TEST_TIMES; i++) {
    EXPECT_EQ(ListIter_next(&list_iter_), nullptr);
  }
}

TEST_F(ListIterTest, CtorReset) {
  for (int i = 0; i < NUM_LIST_ITEM; i++) {
    EXPECT_EQ(ListIter_next(&list_iter_), (void*)i);
  }

  ListIter_ctor(&list_iter_, &list_);
  for (int i = 0; i < NUM_LIST_ITEM; i++) {
    EXPECT_EQ(ListIter_next(&list_iter_), (void*)i);
  }
}

/* task initialization test --------------------------------------------------*/
TEST(TaskInitTest, TaskCtor) {
  Task task;
  // fake task_code function pointer
  TaskFunction_t task_code = (TaskFunction_t)0x12345678;

  Task_ctor(&task, task_code);
  EXPECT_EQ(task.state_, TaskReset);
  EXPECT_EQ(task.task_code_, task_code);
}

/* task test -----------------------------------------------------------------*/
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
