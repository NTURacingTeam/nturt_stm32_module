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

/* type ----------------------------------------------------------------------*/
typedef enum module_ret {
  MODULE_OK = 0,
  MODULE_ERROR,
  MODULE_TIMEOUT,
  MODULE_BUSY,
} ModuleRet;

typedef enum task_state {
  TASK_RESET = 0,
  TASK_RUNNING,
} TaskState;

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
#define IS_LESS(VAL1, VAL2) ((VAL1) < (VAL2))

/* function ------------------------------------------------------------------*/
void __module_assert_fail(const char* assertion, const char* file,
                          unsigned int line, const char* function);

/* abstract class ------------------------------------------------------------*/
struct TaskVtbl;

/**
 * @brief Abstract class for managing freertos task.
 *
 */
typedef struct task {
  // virtual table
  struct TaskVtbl* vptr;

  // member variable
  TaskState state_;

  TaskHandle_t task_handle_;

  StaticTask_t task_control_block_;

  void (*task_code_)(void*);
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
void Task_ctor(Task* const self, void (*const task_code)(void*));

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
 */
void Task_create_freertos_task(Task* const self, const char* const task_name,
                               UBaseType_t task_priority,
                               StackType_t* const stack,
                               const uint32_t stack_size);

#ifdef __cplusplus
}
#endif

#endif  // MODULE_COMMON_H
