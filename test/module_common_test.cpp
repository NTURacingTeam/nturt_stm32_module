// stl incldue
#include <algorithm>
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

using ::testing::_;
using ::testing::ArrayWithSize;
using ::testing::Invoke;
using ::testing::Test;
using ::testing::WithArg;

/* test parameters -----------------------------------------------------------*/
// should be even number
#define NUM_QUEUE_ITEM 10
#define NUM_QUEUE_ITEM_OVER (2 * NUM_QUEUE_ITEM)
#define QUEUE_ELEMENT_TYPE uint32_t
#define NUM_LIST_ITEM 10
#define REPEATED_TEST_TIMES 10

#if 0

/* queue test ----------------------------------------------------------------*/
void Queue_print_queue(Queue* const self) {
  printf("Queue size: %d, capacity: %d, head: %d, tail: %d\n", self->size_,
         self->capacity_, self->head_, self->tail_);
  for (int i = 0; i < self->capacity_; i++) {
    printf("%d ", ((QUEUE_ELEMENT_TYPE*)self->buffer_)[i]);
  }
  printf("\n");
}

/* queue initialization test -------------------------------------------------*/
TEST(QueueInitTest, QueueCtor) {
  Queue queue;
  QUEUE_ELEMENT_TYPE queue_buffer;

  Queue_ctor(&queue, &queue_buffer, NUM_QUEUE_ITEM, sizeof(QUEUE_ELEMENT_TYPE));

  EXPECT_EQ(Queue_get_size(&queue), 0);
  EXPECT_EQ(Queue_get_capacity(&queue), NUM_QUEUE_ITEM);
}

/* queue enqueue dequeue test
 * ------------------------------------------------*/
class QueueEnqueueDequeueTest : public Test {
 protected:
  void SetUp() override {
    Queue_ctor(&queue_, queue_buffer_, NUM_QUEUE_ITEM,
               sizeof(QUEUE_ELEMENT_TYPE));

    for (int i = 0; i < NUM_QUEUE_ITEM; i++) {
      elements_[i] = i;
    }
    for (int i = 0; i < NUM_QUEUE_ITEM_OVER; i++) {
      elements_over_[i] = i;
    }
  }

  Queue queue_;

  // initialize to 0 to avoid shadowing from previous tests
  QUEUE_ELEMENT_TYPE queue_buffer_[NUM_QUEUE_ITEM] = {0},
                     elements_[NUM_QUEUE_ITEM],
                     elements_over_[NUM_QUEUE_ITEM_OVER],
                     elements_out_[NUM_QUEUE_ITEM] = {0};
};

TEST_F(QueueEnqueueDequeueTest, EnqueueDequeue) {
  for (int i = 0; i < NUM_QUEUE_ITEM; i++) {
    Queue_enqueue(&queue_, &elements_[i]);
  }
  for (int i = 0; i < NUM_QUEUE_ITEM; i++) {
    EXPECT_EQ(Queue_dequeue(&queue_, &elements_out_[i]), 1);
  }

  EXPECT_THAT((QUEUE_ELEMENT_TYPE*)elements_,
              ArrayWithSize(elements_out_, NUM_QUEUE_ITEM));
}

TEST_F(QueueEnqueueDequeueTest, EnqueueAll) {
  Queue_enqueue_all(&queue_, elements_, NUM_QUEUE_ITEM);
  for (int i = 0; i < NUM_QUEUE_ITEM; i++) {
    EXPECT_EQ(Queue_dequeue(&queue_, &elements_out_[i]), 1);
  }

  EXPECT_THAT((QUEUE_ELEMENT_TYPE*)elements_,
              ArrayWithSize(elements_out_, NUM_QUEUE_ITEM));
}

TEST_F(QueueEnqueueDequeueTest, DequeueAll) {
  Queue_enqueue_all(&queue_, elements_, NUM_QUEUE_ITEM);
  Queue_dequeue_all(&queue_, elements_out_, NUM_QUEUE_ITEM);

  EXPECT_THAT((QUEUE_ELEMENT_TYPE*)elements_,
              ArrayWithSize(elements_out_, NUM_QUEUE_ITEM));
}

TEST_F(QueueEnqueueDequeueTest, EnqueueAllWithLengthSmallerThanCapacity) {
  Queue_enqueue_all(&queue_, elements_, NUM_QUEUE_ITEM / 2);
  Queue_enqueue_all(&queue_, &elements_[NUM_QUEUE_ITEM / 2],
                    NUM_QUEUE_ITEM / 2);
  Queue_dequeue_all(&queue_, elements_out_, NUM_QUEUE_ITEM);

  EXPECT_THAT((QUEUE_ELEMENT_TYPE*)elements_,
              ArrayWithSize(elements_out_, NUM_QUEUE_ITEM));
}

TEST_F(QueueEnqueueDequeueTest, DequeueAllWithLengthSmallerThanCapacity) {
  Queue_enqueue_all(&queue_, elements_, NUM_QUEUE_ITEM);
  Queue_dequeue_all(&queue_, elements_out_, NUM_QUEUE_ITEM / 2);
  Queue_dequeue_all(&queue_, &elements_out_[NUM_QUEUE_ITEM / 2],
                    NUM_QUEUE_ITEM / 2);

  EXPECT_THAT((QUEUE_ELEMENT_TYPE*)elements_,
              ArrayWithSize(elements_out_, NUM_QUEUE_ITEM));
}

TEST_F(QueueEnqueueDequeueTest, EnqueueDequeueLoopAroundBuffer) {
  Queue_enqueue_all(&queue_, elements_, NUM_QUEUE_ITEM / 2);
  Queue_dequeue_all(&queue_, elements_out_, NUM_QUEUE_ITEM / 2);
  for (int i = 0; i < NUM_QUEUE_ITEM; i++) {
    Queue_enqueue(&queue_, &elements_[i]);
  }
  for (int i = 0; i < NUM_QUEUE_ITEM; i++) {
    Queue_dequeue(&queue_, &elements_out_[i]);
  }

  EXPECT_THAT((QUEUE_ELEMENT_TYPE*)elements_,
              ArrayWithSize(elements_out_, NUM_QUEUE_ITEM));
}

TEST_F(QueueEnqueueDequeueTest, EnqueueAllLoopAroundBuffer) {
  Queue_enqueue_all(&queue_, elements_, NUM_QUEUE_ITEM / 2);
  Queue_dequeue_all(&queue_, elements_out_, NUM_QUEUE_ITEM / 2);
  Queue_enqueue_all(&queue_, elements_, NUM_QUEUE_ITEM);
  for (int i = 0; i < NUM_QUEUE_ITEM; i++) {
    Queue_dequeue(&queue_, &elements_out_[i]);
  }

  EXPECT_THAT((QUEUE_ELEMENT_TYPE*)elements_,
              ArrayWithSize(elements_out_, NUM_QUEUE_ITEM));
}

TEST_F(QueueEnqueueDequeueTest, DequeueAllLoopAroundBuffer) {
  Queue_enqueue_all(&queue_, elements_, NUM_QUEUE_ITEM / 2);
  Queue_dequeue_all(&queue_, elements_out_, NUM_QUEUE_ITEM / 2);
  Queue_enqueue_all(&queue_, elements_, NUM_QUEUE_ITEM);
  Queue_dequeue_all(&queue_, elements_out_, NUM_QUEUE_ITEM);

  EXPECT_THAT((QUEUE_ELEMENT_TYPE*)elements_,
              ArrayWithSize(elements_out_, NUM_QUEUE_ITEM));
}

TEST_F(QueueEnqueueDequeueTest, EnqueueOverCapacity) {
  for (int i = 0; i < NUM_QUEUE_ITEM_OVER; i++) {
    Queue_enqueue(&queue_, &elements_over_[i]);
  }
  Queue_dequeue_all(&queue_, elements_out_, NUM_QUEUE_ITEM);

  // the first NUM_QUEUE_ITEM_OVER - NUM_QUEUE_ITEM elements should be
  // overwritten
  EXPECT_THAT((QUEUE_ELEMENT_TYPE*)&elements_over_[NUM_QUEUE_ITEM_OVER -
                                                   NUM_QUEUE_ITEM],
              ArrayWithSize(elements_out_, NUM_QUEUE_ITEM));
}

TEST_F(QueueEnqueueDequeueTest, DequeueEmpty) {
  QUEUE_ELEMENT_TYPE element_out = 0x12345678;
  EXPECT_EQ(Queue_dequeue(&queue_, &element_out), 0);
  EXPECT_EQ(element_out, 0x12345678);
}

TEST_F(QueueEnqueueDequeueTest, EnqueueAllOverCapacity) {
  Queue_enqueue_all(&queue_, elements_over_, NUM_QUEUE_ITEM_OVER);
  Queue_dequeue_all(&queue_, elements_out_, NUM_QUEUE_ITEM);

  // the first NUM_QUEUE_ITEM_OVER - NUM_QUEUE_ITEM elements should be
  // overwritten
  EXPECT_THAT((QUEUE_ELEMENT_TYPE*)&elements_over_[NUM_QUEUE_ITEM_OVER -
                                                   NUM_QUEUE_ITEM],
              ArrayWithSize(elements_out_, NUM_QUEUE_ITEM));
}

TEST_F(QueueEnqueueDequeueTest, DequeueAllEmpty) {
  QUEUE_ELEMENT_TYPE elements_out[NUM_QUEUE_ITEM];
  std::fill(elements_out, elements_out + NUM_QUEUE_ITEM, 0x12345678);

  Queue_dequeue_all(&queue_, elements_out, NUM_QUEUE_ITEM);

  EXPECT_THAT((QUEUE_ELEMENT_TYPE*)elements_out,
              ArrayWithSize(elements_out, NUM_QUEUE_ITEM));
}

TEST_F(QueueEnqueueDequeueTest, RepeatedEnqueueDequeue) {
  for (int i = 0; i < REPEATED_TEST_TIMES; i++) {
    for (int j = 0; j < NUM_QUEUE_ITEM - 1; j++) {
      Queue_enqueue(&queue_, &elements_[j]);
    }
    for (int j = 0; j < NUM_QUEUE_ITEM - 1; j++) {
      Queue_dequeue(&queue_, &elements_out_[j]);
    }
    EXPECT_THAT((QUEUE_ELEMENT_TYPE*)elements_,
                ArrayWithSize(elements_out_, NUM_QUEUE_ITEM - 1));
  }
}

TEST_F(QueueEnqueueDequeueTest, RepeatedEnqueueAllDequeueAll) {
  for (int i = 0; i < REPEATED_TEST_TIMES; i++) {
    Queue_enqueue_all(&queue_, elements_, NUM_QUEUE_ITEM - 1);
    Queue_dequeue_all(&queue_, elements_out_, NUM_QUEUE_ITEM - 1);
    EXPECT_THAT((QUEUE_ELEMENT_TYPE*)elements_,
                ArrayWithSize(elements_out_, NUM_QUEUE_ITEM - 1));
  }
}

#endif

/* list initialization test --------------------------------------------------*/
TEST(ListInitTest, ListCtor) {
  List list;

  List_ctor(&list);

  // really nothing to test
}

/* list no item test ---------------------------------------------------------*/
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
