#include "stm32_module/module_common.h"

// glibc include
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// freertos include
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

/* function ------------------------------------------------------------------*/
__attribute__((weak)) void __module_assert_fail(const char *assertion,
                                                const char *file,
                                                unsigned int line,
                                                const char *function) {
  (void)assertion;
  (void)file;
  (void)line;
  (void)function;
}

#if 0

/* constructor ---------------------------------------------------------------*/
void Queue_ctor(Queue *const self, void *buffer, int capacity,
                int element_size) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(buffer));
  module_assert(IS_POSTIVE(capacity));
  module_assert(IS_POSTIVE(element_size));
  module_assert(IS_GREATER_OR_EQUAL(capacity, element_size));

  self->buffer_ = buffer;
  self->capacity_ = capacity;
  self->element_size_ = element_size;
  self->size_ = 0;
  self->head_ = 0;
  self->tail_ = 0;
}

/* member function -----------------------------------------------------------*/
int Queue_get_size(const Queue *const self) {
  module_assert(IS_NOT_NULL(self));
  return self->size_;
}

int Queue_get_capacity(const Queue *const self) {
  module_assert(IS_NOT_NULL(self));
  return self->capacity_;
}

void Queue_enqueue(Queue *const self, const void *const data) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(data));

  // copy data to buffer
  memcpy(self->buffer_ + self->tail_ * self->element_size_, data,
         self->element_size_);
  self->tail_ = (self->tail_ + 1) % self->capacity_;

  // dequeue if buffer is full
  if (self->size_ < self->capacity_) {
    self->size_++;
  } else {
    self->head_ = (self->head_ + 1) % self->capacity_;
  }
}

void Queue_enqueue_all(Queue *const self, const void *send_buffer,
                       int send_buffer_length) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(send_buffer));
  module_assert(IS_POSTIVE(send_buffer_length));

  // check if the send buffer length is larger than the queue capacity
  if (send_buffer_length > self->capacity_) {
    send_buffer += (send_buffer_length - self->capacity_) * self->element_size_;
    send_buffer_length = self->capacity_;
  }

  // check if buffer will loop around
  if (self->tail_ + send_buffer_length > self->capacity_) {
    int num_items_before_wrap = self->capacity_ - self->tail_;
    memcpy(self->buffer_ + self->tail_ * self->element_size_, send_buffer,
           num_items_before_wrap * self->element_size_);
    memcpy(self->buffer_,
           send_buffer + num_items_before_wrap * self->element_size_,
           (send_buffer_length - num_items_before_wrap) * self->element_size_);
  } else {
    memcpy(self->buffer_ + self->tail_ * self->element_size_, send_buffer,
           send_buffer_length * self->element_size_);
  }
  self->tail_ = (self->tail_ + send_buffer_length) % self->capacity_;

  // dequeue if buffer is full
  if (self->size_ + send_buffer_length <= self->capacity_) {
    self->size_ += send_buffer_length;
  } else {
    self->size_ = self->capacity_;
    self->head_ = (self->tail_ + 1) % self->capacity_;
  }
}

int Queue_dequeue(Queue *const self, void *const data) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(data));

  if (self->size_ == 0) {
    return 0;
  }

  // copy data from buffer
  memcpy(data, self->buffer_ + self->head_ * self->element_size_,
         self->element_size_);
  self->head_ = (self->head_ + 1) % self->capacity_;
  self->size_--;
  return 1;
}

int Queue_dequeue_all(Queue *const self, void *const receive_buffer,
                      const int receive_buffer_length) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(receive_buffer));
  module_assert(IS_POSTIVE(receive_buffer_length));

  if (self->size_ == 0) {
    return 0;
  }

  int num_items =
      receive_buffer_length > self->size_ ? self->size_ : receive_buffer_length;
  // check if buffer will loop around
  if (self->head_ + num_items > self->capacity_) {
    int num_items_before_wrap = self->capacity_ - self->head_;
    memcpy(receive_buffer, self->buffer_ + self->head_ * self->element_size_,
           num_items_before_wrap * self->element_size_);
    memcpy(receive_buffer + num_items_before_wrap * self->element_size_,
           self->buffer_,
           (num_items - num_items_before_wrap) * self->element_size_);
  } else {
    memcpy(receive_buffer, self->buffer_ + self->head_ * self->element_size_,
           num_items * self->element_size_);
  }

  self->head_ = (self->head_ + num_items) % self->capacity_;
  self->size_ -= num_items;
  return num_items;
}

#endif

/* constructor ---------------------------------------------------------------*/
void List_ctor(List *const self) {
  module_assert(IS_NOT_NULL(self));

  self->head_ = NULL;
  self->end_ = NULL;
  self->index_ = NULL;
  self->size_ = 0;
}

/* member function -----------------------------------------------------------*/
int List_size(List *const self) { return self->size_; }

void List_push_back(List *const self, struct list_cb *const list_cb,
                    void *const data) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(list_cb));
  module_assert(IS_NOT_NULL(data));

  // create new list data
  list_cb->data = data;
  list_cb->next = NULL;

  // add data to list
  if (self->head_ == NULL) {
    self->head_ = list_cb;
    self->end_ = list_cb;
  } else {
    self->end_->next = list_cb;
    self->end_ = list_cb;
  }

  self->size_++;
}

void *List_at(List *const self, const int index) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NEGATIVE(index));

  if (index >= self->size_) {
    return NULL;
  }

  struct list_cb *list_cb = self->head_;
  for (int i = 0; i < index; i++) {
    list_cb = list_cb->next;
  }

  return list_cb->data;
}

/* constructor ---------------------------------------------------------------*/
void ListIter_ctor(ListIter *const self, List *const list) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(list));

  self->index_ = list->head_;
}

/* member function -----------------------------------------------------------*/
void *ListIter_next(ListIter *const self) {
  module_assert(IS_NOT_NULL(self));

  if (self->index_ == NULL) {
    return NULL;
  }

  void *const data = self->index_->data;
  self->index_ = self->index_->next;
  return data;
}

/* constructor ---------------------------------------------------------------*/
void SharedResource_ctor(SharedResource *const self, void *const resource) {
  module_assert(IS_NOT_NULL(self));

  // initialize member variable
  self->resource_ = resource;
  self->mutex_handle_ =
      xSemaphoreCreateMutexStatic(&self->mutex_control_block_);
}

/* member function -----------------------------------------------------------*/
void *SharedResource_access(SharedResource *const self) {
  module_assert(IS_NOT_NULL(self));

  xSemaphoreTake(self->mutex_handle_, portMAX_DELAY);
  return self->resource_;
}

void SharedResource_end_access(SharedResource *const self) {
  module_assert(IS_NOT_NULL(self));

  xSemaphoreGive(self->mutex_handle_);
}

/* virtual function redirection ----------------------------------------------*/
inline ModuleRet Task_start(Task *const self) {
  return self->vptr_->start(self);
}

/* virtual function definition -----------------------------------------------*/
// pure virtual function for Task base class
ModuleRet __Task_start(Task *const self) {
  (void)self;

  module_assert(0);
  return ModuleError;
}

/* constructor ---------------------------------------------------------------*/
void Task_ctor(Task *const self, TaskFunction_t task_code) {
  module_assert(IS_NOT_NULL(self));

  // assign base virtual function
  static struct TaskVtbl vtbl_base = {
      .start = __Task_start,
  };
  self->vptr_ = &vtbl_base;

  // initialize member variable
  self->state_ = TaskReset;
  self->task_handle_ = NULL;
  self->task_code_ = task_code;
}

/* member function -----------------------------------------------------------*/
ModuleRet Task_create_freertos_task(Task *const self,
                                    const char *const task_name,
                                    UBaseType_t task_priority,
                                    StackType_t *const stack,
                                    const uint32_t stack_size) {
  if (self->state_ == TaskRunning) {
    return ModuleBusy;
  }

  self->task_handle_ =
      xTaskCreateStatic(self->task_code_, task_name, stack_size, (void *)self,
                        task_priority, stack, &self->task_control_block_);
  self->state_ = TaskRunning;
  return ModuleOK;
}

ModuleRet Task_delete(Task *const self) {
  if (self->state_ == TaskReset) {
    return ModuleError;
  }

  vTaskDelete(self->task_handle_);
  self->state_ = TaskReset;
  return ModuleOK;
}
