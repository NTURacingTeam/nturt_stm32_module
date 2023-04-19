/**
 * @file module_common.h
 * @author QuantumSpawner jet22854111@gmail.com
 * @brief Common header for all modules.
 */

#ifndef MODULE_COMMON_H
#define MODULE_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

// glibc include
#include <stddef.h>

// freertos include
#include "FreeRTOS.h"
#include "task.h"

/* macro ---------------------------------------------------------------------*/
// used for stoping mocked freertos delay function
#define MODULE_END -1

// assert macro for replacing the glibc one defined in assert.h for smaller
// flash and sram footprint
#if defined(DEBUG) || defined(TESTING)
#define module_assert(expr)                                      \
  do {                                                           \
    if (!(expr)) {                                               \
      __module_assert_fail(#expr, __FILE__, __LINE__, __func__); \
    }                                                            \
  } while (0)
#else
#define module_assert(expr) (void)(expr)
#endif  // defined (DEBUG) || defined (TESTING)

// assert macro
#define IS_NOT_NULL(PTR) ((PTR) != NULL)
#define IS_POSTIVE(VAL) ((VAL) > 0)
#define IS_NOT_NEGATIVE(VAL) ((VAL) >= 0)
#define IS_GREATER(VAL1, VAL2) ((VAL1) > (VAL2))
#define IS_GREATER_OR_EQUAL(VAL1, VAL2) ((VAL1) >= (VAL2))
#define IS_LESS(VAL1, VAL2) ((VAL1) < (VAL2))
#define IS_LESS_OR_EQUAL(VAL1, VAL2) ((VAL1) <= (VAL2))

/* type ----------------------------------------------------------------------*/
typedef enum module_ret {
  ModuleOK = 0,
  ModuleError,
  ModuleTimeout,
  ModuleBusy,
} ModuleRet;

typedef enum task_priority {
  TaskPriorityNone = 0,
  TaskPriorityIdle,
  TaskPriorityLowest,
  TaskPriorityLow,
  TaskPriorityBelowNormal,
  TaskPriorityNormal,
  TaskPriorityAboveNormal,
  TaskPriorityHigh,
  TaskPriorityRealtime,
} TaskPriority;

typedef enum task_state {
  TaskReset = 0,
  TaskRunning,
} TaskState;

/// @brief Struct for list control block.
struct list_cb {
  void* data;

  struct list_cb* volatile next;
};

/* function ------------------------------------------------------------------*/
void __module_assert_fail(const char* assertion, const char* file,
                          unsigned int line, const char* function);

#if 0

/* class ---------------------------------------------------------------------*/
/**
 * @brief Class for managing queue.
 *
 * @note The data is copied to the queue.
 */
typedef struct queue {
  void* volatile buffer_;

  volatile int capacity_;

  volatile int element_size_;

  volatile int size_;

  volatile int head_;

  volatile int tail_;
} Queue;

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for Queue.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] buffer The buffer for the queue.
 * @param[in] capacity The capacity of the queue.
 * @param[in] element_size The size of the element.
 * @note User is responsible for allocating memory for buffer.
 * @return None.
 */
void Queue_ctor(Queue* const self, void* buffer, int capacity,
                int element_size);

/* member function -----------------------------------------------------------*/
/**
 * @brief Function to get the size of the queue.
 *
 * @param[in,out] self The instance of the class.
 * @return int The size of the queue.
 */
int Queue_get_size(const Queue* const self);

/**
 * @brief Function to get the capacity of the queue.
 *
 * @param[in,out] self The instance of the class.
 * @return int The capacity of the queue.
 */
int Queue_get_capacity(const Queue* const self);

/**
 * @brief Function to put an element to the queue.
 *
 * @param[in,out] self The instance of the class.
 * @return None.
 * @warning This function is not thread safe.
 */
void Queue_enqueue(Queue* const self, const void* const data);

void Queue_enqueue_all(Queue* const self, const void* send_buffer,
                       int send_buffer_length);

/**
 * @brief Function to get an element from the queue.
 *
 * @param[in,out] self The instance of the class.
 * @param[out] data The data to get.
 * @return int Number of elements received.
 * @warning This function is not thread safe.
 */
int Queue_dequeue(Queue* const self, void* const data);

/**
 * @brief Function to get all elements from the queue.
 *
 * @param[in,out] self The instance of the class.
 * @param[out] receive_buffer The buffer to receive data.
 * @param[in] receive_buffer_length The length of the receive buffer.
 * @return int Number of elements received.
 * @warning This function is not thread safe.
 */
int Queue_dequeue_all(Queue* const self, void* const receive_buffer,
                      const int receive_buffer_length);

#endif

/* class ---------------------------------------------------------------------*/
/**
 * @brief Class for managing list.
 *
 * @warning The data is not copied, only the pointer is stored, so user is
 * resposible for managing the lifetime of the data.
 * @note Implemented as a singly linked list.
 *
 */
typedef struct list {
  struct list_cb* volatile head_;

  struct list_cb* volatile end_;

  struct list_cb* volatile index_;

  volatile int size_;
} List;

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for List.
 *
 * @param[in,out] self The instance of the class.
 * @return None.
 */
void List_ctor(List* const self);

/* member function -----------------------------------------------------------*/
/**
 * @brief Function to get the size of the list.
 *
 * @param[in,out] self The instance of the class.
 * @return int The size of the list.
 */
int List_size(List* const self);

/**
 * @brief Function to add an item to the end of the list.
 *
 * @param[in,out] self The instance of the class.
 * @param[in,out] list_cb Control block for the list data.
 * @param[in] data The data to add.
 * @return None.
 * @warning This function is not thread safe.
 * @note User is resposible for allocating memory for list_cb.
 */
void List_push_back(List* const self, struct list_cb* const list_cb,
                    void* const data);

/**
 * @brief Function to access the data at the index.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] index The index of the data.
 * @return void* The data at the index, NULL if the index is out of range.
 * @warning This function is not thread safe.
 */
void* List_at(List* const self, const int index);

/* class ---------------------------------------------------------------------*/
/**
 * @brief Class for iteratoring through list.
 *
 */
typedef struct list_iter {
  struct list_cb* volatile index_;
} ListIter;

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for ListIter, can also be used to reset the iterator.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] list The list to iterate through.
 * @return None.
 */
void ListIter_ctor(ListIter* const self, List* const list);

/* member function -----------------------------------------------------------*/
/**
 * @brief Function to iterate to the next data the list.
 *
 * @param[in,out] self The instance of the class.
 * @return void* The next data of the list, NULL if the whole list has been
 * iterated through.
 * @warning This function is not thread safe.
 */
void* ListIter_next(ListIter* const self);

/* abstract class ------------------------------------------------------------*/
// forward declaration
struct TaskVtbl;

/**
 * @brief Abstract class for managing freertos task.
 *
 */
typedef struct task {
  // virtual table
  struct TaskVtbl* vptr_;

  // member variable
  TaskState state_;

  TaskHandle_t task_handle_;

  StaticTask_t task_control_block_;

  TaskFunction_t task_code_;
} Task;

/// @brief Virtual table for Task.
struct TaskVtbl {
  ModuleRet (*start)(Task*);
};

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for Task.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] task_stack The stack of the freertos task.
 * @param[in] task_code The function to run in freertos.
 * @return None.
 */
void Task_ctor(Task* const self, TaskFunction_t task_code);

/* member function -----------------------------------------------------------*/
/**
 * @brief Start the freertos task.
 *
 * @param[in,out] self The instance of the class.
 * @return ModuleRet Error code.
 * @note This function is virtual.
 */
ModuleRet Task_start(Task* const self);

/**
 * @brief Function for creating freertos task.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] task_name The name of the task.
 * @param[in] task_priority The priority of the task.
 * @param[in] stack The stack for the task.
 * @param[in] stack_size The size of the stack in words.
 * @return ModuleRet Error code.
 */
ModuleRet Task_create_freertos_task(Task* const self,
                                    const char* const task_name,
                                    UBaseType_t task_priority,
                                    StackType_t* const stack,
                                    const uint32_t stack_size);

/**
 * @brief Function for deleting the freertos task.
 *
 * @param[in,out] self The instance of the class.
 * @return ModuleRet Error code.
 */
ModuleRet Task_delete(Task* const self);

#ifdef __cplusplus
}
#endif

#endif  // MODULE_COMMON_H
