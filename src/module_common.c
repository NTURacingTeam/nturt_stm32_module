#include "stm32_module/module_common.h"

// glibc include
#include <stddef.h>
#include <stdint.h>

// freertos include
#include "FreeRTOS.h"
#include "task.h"

/* function ------------------------------------------------------------------*/
void __module_assert_fail(const char *assertion, const char *file,
                          unsigned int line, const char *function) {
  (void)assertion;
  (void)file;
  (void)line;
  (void)function;
}

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
