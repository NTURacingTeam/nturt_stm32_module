#include "stm32_module/can_transceiver.h"

// glibc include
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// stm32 include
#include "stm32_module/stm32_hal.h"

// freertos include
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* static variable -----------------------------------------------------------*/
/// @brief List for tracking the list of can transceivers for using in high
/// priority message callback.
static List can_transceiver_list;

/**
 * @brief Flag for checking if this is the first can transceiver for
 * initializing can_transceiver_list.
 *
 * This variable is not set to static since it has to be reset to true for
 * testing.
 */
/*static*/ bool is_first_can_transceiver = true;

/* virtual function redirection ----------------------------------------------*/
inline ModuleRet CanTransceiver_start(CanTransceiver* const self) {
  return self->super_.vptr_->start((Task*)self);
}

inline ModuleRet CanTransceiver_configure(CanTransceiver* const self) {
  return self->vptr_->configure(self);
}

inline ModuleRet CanTransceiver_receive(CanTransceiver* const self,
                                        const bool is_extended,
                                        const uint32_t id, const uint8_t dlc,
                                        const uint8_t* const data) {
  return self->vptr_->receive(self, is_extended, id, dlc, data);
}

inline ModuleRet CanTransceiver_receive_hp(CanTransceiver* const self,
                                           const bool is_extended,
                                           const uint32_t id, const uint8_t dlc,
                                           const uint8_t* const data) {
  return self->vptr_->receive_hp(self, is_extended, id, dlc, data);
}

inline ModuleRet CanTransceiver_periodic_update(CanTransceiver* const self,
                                                const TickType_t current_tick) {
  return self->vptr_->periodic_update(self, current_tick);
}

/* virtual function definition -----------------------------------------------*/
// from Task base class
ModuleRet __CanTransceiver_start(Task* const _self) {
  module_assert(IS_NOT_NULL(_self));

  CanTransceiver* const self = (CanTransceiver*)_self;
  if (self->super_.state_ == TaskRunning) {
    return ModuleBusy;
  }

  CanTransceiver_configure(self);
  Task_create_freertos_task((Task*)self, "can_transceiver",
                            TaskPriorityAboveNormal, self->task_stack_,
                            sizeof(self->task_stack_) / sizeof(StackType_t));
  return ModuleOK;
}

// default to do nothing
ModuleRet __CanTransceiver_configure(CanTransceiver* const self) {
  (void)self;
  return ModuleOK;
}

// pure virtual function for CanTransceiver base class
ModuleRet __CanTransceiver_receive(CanTransceiver* const self,
                                   const bool is_extended, const uint32_t id,
                                   const uint8_t dlc,
                                   const uint8_t* const data) {
  (void)self;
  (void)is_extended;
  (void)id;
  (void)dlc;
  (void)data;

  module_assert(0);
  return ModuleError;
}

// pure virtual function for CanTransceiver base class
ModuleRet __CanTransceiver_receive_hp(CanTransceiver* const self,
                                      const bool is_extended, const uint32_t id,
                                      const uint8_t dlc,
                                      const uint8_t* const data) {
  (void)self;
  (void)is_extended;
  (void)id;
  (void)dlc;
  (void)data;

  module_assert(0);
  return ModuleError;
}

// pure virtual function for CanTransceiver base class
ModuleRet __CanTransceiver_periodic_update(CanTransceiver* const self,
                                           const TickType_t current_tick) {
  (void)self;
  (void)current_tick;

  module_assert(0);
  return ModuleError;
}

/* constructor ---------------------------------------------------------------*/
void CanTransceiver_ctor(CanTransceiver* const self,
                         CanHandle* const can_handle) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(can_handle));

  // construct inherited class and redirect virtual function
  Task_ctor(&self->super_, CanTransceiver_task_code);
  static struct TaskVtbl vtbl = {
      .start = __CanTransceiver_start,
  };
  self->super_.vptr_ = &vtbl;

  // assign base virtual function
  static struct CanTransceiverVtbl vtbl_base = {
      .configure = __CanTransceiver_configure,
      .receive = __CanTransceiver_receive,
      .receive_hp = __CanTransceiver_receive_hp,
      .periodic_update = __CanTransceiver_periodic_update,
  };
  self->vptr_ = &vtbl_base;

  // initialize member variable
  self->can_handle_ = can_handle;
  if (is_first_can_transceiver) {
    List_ctor(&can_transceiver_list);
    is_first_can_transceiver = false;
  }
  List_push_back(&can_transceiver_list, &self->can_transceiver_list_cb,
                 (void*)self);
}

/* member function -----------------------------------------------------------*/
ModuleRet CanTransceiver_transmit(CanTransceiver* const self,
                                  const bool is_extended, const uint32_t id,
                                  const uint8_t dlc, uint8_t* const data) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_DLC(dlc));
  module_assert(IS_NOT_NULL(data));

  if (self->super_.state_ != TaskRunning) {
    return ModuleError;
  }

#if defined(HAL_CAN_MODULE_ENABLED)
  static CAN_TxHeaderTypeDef tx_header = {
      .IDE = CAN_ID_STD, .RTR = CAN_RTR_DATA, .TransmitGlobalTime = DISABLE};
  tx_header.DLC = dlc;
  if (is_extended) {
    tx_header.IDE = CAN_ID_EXT;
    tx_header.ExtId = id;
  } else {
    tx_header.IDE = CAN_ID_STD;
    tx_header.StdId = id;
  }
  uint32_t tx_mailbox;
  if (HAL_CAN_AddTxMessage(self->can_handle_, &tx_header, data, &tx_mailbox) !=
      HAL_OK) {
    return ModuleError;
  }
#elif defined(HAL_FDCAN_MODULE_ENABLED)
  static FDCAN_TxHeaderTypeDef tx_header = {
      .TxFrameType = FDCAN_DATA_FRAME,
      .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
      .BitRateSwitch = FDCAN_BRS_OFF,
      .FDFormat = FDCAN_FD_CAN,
      .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
      .MessageMarker = 0};
  tx_header.IdType = is_extended ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID;
  tx_header.Identifier = id;
  tx_header.DataLength = dlc << 16;
  if (HAL_FDCAN_AddMessageToTxFifoQ(self->can_handle_, &tx_header, data) !=
      HAL_OK) {
    return ModuleError;
  }
#endif

  return ModuleOK;
}

void CanTransceiver_task_code(void* const _self) {
  CanTransceiver* const self = (CanTransceiver*)_self;
  TickType_t last_wake = xTaskGetTickCount();

  while (1) {
    // receive and decode can signal
#if defined(HAL_CAN_MODULE_ENABLED)
    uint32_t fifo_level =
        HAL_CAN_GetRxFifoFillLevel(self->can_handle_, CAN_RX_FIFO0);
    for (uint32_t i = 0; i < fifo_level; i++) {
      CAN_RxHeaderTypeDef rx_header;
      uint8_t rx_data[8];
      HAL_CAN_GetRxMessage(self->can_handle_, CAN_RX_FIFO0, &rx_header,
                           rx_data);
      CanTransceiver_receive(
          self, rx_header.IDE == CAN_ID_EXT,
          rx_header.IDE == CAN_ID_EXT ? rx_header.ExtId : rx_header.StdId,
          rx_header.DLC, rx_data);
    }
#elif defined(HAL_FDCAN_MODULE_ENABLED)
    uint32_t fifo_level =
        HAL_FDCAN_GetRxFifoFillLevel(self->can_handle_, FDCAN_RX_FIFO0);
    for (uint32_t i = 0; i < fifo_level; i++) {
      FDCAN_RxHeaderTypeDef rx_header;
      uint8_t rx_data[8];
      HAL_FDCAN_GetRxMessage(self->can_handle_, FDCAN_RX_FIFO0, &rx_header,
                             rx_data);
      CanTransceiver_receive(self, rx_header.IdType, rx_header.Identifier,
                             rx_header.DataLength >> 16, rx_data);
    }
#endif

    // periodic update for checking timeout and transmit can signal, etc.
    CanTransceiver_periodic_update(self, last_wake);

    vTaskDelayUntil(&last_wake, 5);
  }
}

/* constructor ---------------------------------------------------------------*/
void CanFrame_ctor(CanFrame* const self, void* const frame) {
  module_assert(IS_NOT_NULL(self));

  // initialize member variable
  self->frame_ = frame;
  self->mutex_handle_ =
      xSemaphoreCreateMutexStatic(&self->mutex_control_block_);
}

/* member function -----------------------------------------------------------*/
void* CanFrame_access(CanFrame* const self) {
  module_assert(IS_NOT_NULL(self));

  xSemaphoreTake(self->mutex_handle_, portMAX_DELAY);
  return self->frame_;
}

void CanFrame_end_access(CanFrame* const self) {
  module_assert(IS_NOT_NULL(self));

  xSemaphoreGive(self->mutex_handle_);
}

/* static and callback function ----------------------------------------------*/
// freertos deferred interrupt handler for receiving high priority can message
static void received_hp_deferred(void* const _self, const uint32_t argument) {
  (void)argument;

  CanTransceiver* const self = (CanTransceiver*)_self;
#if defined(HAL_CAN_MODULE_ENABLED)
  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];
  if (HAL_CAN_GetRxMessage(self->can_handle_, CAN_RX_FIFO1, &rx_header,
                           rx_data) != HAL_OK) {
    module_assert(0);
  }
  CanTransceiver_receive_hp(
      self, rx_header.IDE == CAN_ID_EXT,
      rx_header.IDE == CAN_ID_EXT ? rx_header.ExtId : rx_header.StdId,
      rx_header.DLC, rx_data);
#elif defined(HAL_FDCAN_MODULE_ENABLED)
  FDCAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];
  if (HAL_FDCAN_GetRxMessage(self->can_handle_, FDCAN_RX_FIFO1, &rx_header,
                             rx_data) != HAL_OK) {
    module_assert(0);
  }
  CanTransceiver_receive_hp(self, rx_header.IdType, rx_header.Identifier,
                            rx_header.DataLength >> 16, rx_data);
#endif
}

// isr from fdcan fx fifo1 for receiving high priority can message
#if defined(HAL_CAN_MODULE_ENABLED)
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef* const hcan) {
  CanTransceiver* transceiver;
  ListIter can_iter;
  ListIter_ctor(&can_iter, &can_transceiver_list);
  do {
    transceiver = (CanTransceiver*)ListIter_next(&can_iter);
    if (transceiver == NULL) {
      module_assert(0);
    }
  } while (transceiver->can_handle_ != hcan);

  BaseType_t require_contex_switch = pdFALSE;
  xTimerPendFunctionCallFromISR(received_hp_deferred, (void*)transceiver, 0,
                                &require_contex_switch);
  portYIELD_FROM_ISR(require_contex_switch);
}
#elif defined(HAL_FDCAN_MODULE_ENABLED)
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef* const hfdcan,
                               uint32_t const RxFifo1ITs) {
  if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET) {
    CanTransceiver* transceiver;
    ListIter can_iter;
    ListIter_ctor(&can_iter, &can_transceiver_list);
    do {
      transceiver = (CanTransceiver*)ListIter_next(&can_iter);
      if (transceiver == NULL) {
        module_assert(0);
      }
    } while (transceiver->can_handle_ != hfdcan);

    BaseType_t require_contex_switch = pdFALSE;
    xTimerPendFunctionCallFromISR(received_hp_deferred, (void*)transceiver, 0,
                                  &require_contex_switch);
    portYIELD_FROM_ISR(require_contex_switch);
  }
}
#endif
