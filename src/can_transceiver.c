#include "stm32_module/can_transceiver.h"

// glibc include
#include <stddef.h>
#include <stdint.h>

// stm32 include
#include "cmsis_os.h"
#if defined(STM32G431xx)
#include "stm32g4xx_hal.h"
#elif defined(STM32H723xx)
#include "stm32h7xx_hal.h"
#endif

// freertos include
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* static variable -----------------------------------------------------------*/
// convenient array for numbering gpio ports
#if defined(STM32G431xx)
#define NUM_FDCAN_INSTANCE 1
static const FDCAN_GlobalTypeDef* fdcan_instance[NUM_FDCAN_INSTANCE] = {FDCAN1};
#elif defined(STM32H723xx)
#define NUM_FDCAN_INSTANCE 3
static const FDCAN_GlobalTypeDef* fdcan_instance[NUM_FDCAN_INSTANCE] = {
    FDCAN1, FDCAN2, FDCAN3};
#endif

static CanTransceiver* transceiver_for_fdcan[NUM_FDCAN_INSTANCE] = {};

static FDCAN_TxHeaderTypeDef tx_header = {
    .TxFrameType = FDCAN_DATA_FRAME,
    .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
    .BitRateSwitch = FDCAN_BRS_OFF,
    .FDFormat = FDCAN_FD_CAN,
    .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
    .MessageMarker = 0};

/* virtual function redirection ----------------------------------------------*/
inline ModuleRet CanTransceiver_start(CanTransceiver* const self) {
  return self->super_.vptr->start((Task*)self);
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
  if (self->super_.state_ == TASK_RUNNING) {
    return MODULE_BUSY;
  }

  CanTransceiver_configure(self);
  Task_create_freertos_task((Task*)self, "can_transceiver",
                            osPriorityAboveNormal, self->task_stack_,
                            sizeof(self->task_stack_) / sizeof(StackType_t));
  return MODULE_OK;
}

// default to do nothing
ModuleRet __CanTransceiver_configure(CanTransceiver* const self) { (void)self; }

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
}

// pure virtual function for CanTransceiver base class
ModuleRet __CanTransceiver_periodic_update(CanTransceiver* const self,
                                           const TickType_t current_tick) {
  (void)self;
  (void)current_tick;

  module_assert(0);
}

/* constructor ---------------------------------------------------------------*/
void CanTransceiver_ctor(CanTransceiver* const self,
                         FDCAN_HandleTypeDef* const can_handle) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(can_handle));

  // construct inherited class and redirect virtual function
  Task_ctor(&self->super_, CanTransceiver_task_code);
  static struct TaskVtbl vtbl = {
      .start = __CanTransceiver_start,
  };
  self->super_.vptr = &vtbl;

  // assign base virtual function
  static struct CanTransceiverVtbl vtbl_base = {
      .receive = __CanTransceiver_receive,
      .receive_hp = __CanTransceiver_receive_hp,
      .periodic_update = __CanTransceiver_periodic_update,
  };
  self->vptr_ = &vtbl_base;

  // initialize member variable
  self->can_handle_ = can_handle;
  for (int i = 0; i < NUM_FDCAN_INSTANCE; i++) {
    if (can_handle->Instance == fdcan_instance[i]) {
      transceiver_for_fdcan[i] = self;
      return;
    }
  }

  module_assert(0);
}

/* member function -----------------------------------------------------------*/
ModuleRet CanTransceiver_transmit(CanTransceiver* const self,
                                  const bool is_extended, const uint32_t id,
                                  const uint8_t dlc, uint8_t* const data) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(data));

  tx_header.IdType = is_extended ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID;
  tx_header.Identifier = id;
  tx_header.DataLength = dlc << 16;

  if (HAL_FDCAN_AddMessageToTxFifoQ(self->can_handle_, &tx_header, data) !=
      HAL_OK) {
    return MODULE_ERROR;
  }
  return MODULE_OK;
}

void CanTransceiver_task_code(void* const _self) {
  CanTransceiver* const self = (CanTransceiver*)_self;
  TickType_t last_wake = xTaskGetTickCount();

  while (1) {
    // receive and decode can signal
    uint32_t fifo_level =
        HAL_FDCAN_GetRxFifoFillLevel(self->can_handle_, FDCAN_RX_FIFO0);
    for (int i = 0; i < fifo_level; i++) {
      FDCAN_RxHeaderTypeDef rx_header;
      uint8_t rx_data[8];
      HAL_FDCAN_GetRxMessage(self->can_handle_, FDCAN_RX_FIFO0, &rx_header,
                             rx_data);
      CanTransceiver_receive(self, rx_header.IdType, rx_header.Identifier,
                             rx_header.DataLength >> 16, rx_data);
    }

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
  FDCAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];
  if (HAL_FDCAN_GetRxMessage(self->can_handle_, FDCAN_RX_FIFO1, &rx_header,
                             rx_data) != HAL_OK) {
    module_assert(0);
  }

  CanTransceiver_receive_hp(self, rx_header.IdType, rx_header.Identifier,
                            rx_header.DataLength >> 16, rx_data);
}

// isr from fdcan fx fifo1 for receiving high priority can message
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef* const hfdcan,
                               uint32_t const RxFifo1ITs) {
  if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET) {
    CanTransceiver* transceiver = NULL;
    for (int i = 0; i < NUM_FDCAN_INSTANCE; i++) {
      if (hfdcan->Instance == fdcan_instance[i] && transceiver_for_fdcan[i]) {
        transceiver = transceiver_for_fdcan[i];
        break;
      }
    }

    if (transceiver == NULL) {
      module_assert(0);
    }

    BaseType_t require_contex_switch = pdFALSE;
    xTimerPendFunctionCallFromISR(received_hp_deferred, (void*)transceiver, 0,
                                  &require_contex_switch);
    portYIELD_FROM_ISR(require_contex_switch);
  }
}
