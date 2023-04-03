#include "mock/can_transceiver_mock.hpp"

// stl include
#include <cstdint>

extern "C" {
// stm32_module include
#include "stm32_module/can_transceiver.h"
#include "stm32_module/module_common.h"
}

// gtest include
#include "cmock/cmock.h"

/* virtual function redirection ----------------------------------------------*/
ModuleRet TestCan_configure(TestCan* const self) {
  return self->super_.vptr_->configure(&self->super_);
}

ModuleRet TestCan_receive(TestCan* const self, const bool is_extended,
                          const uint32_t id, const uint8_t dlc,
                          const uint8_t* const data) {
  return self->super_.vptr_->receive(&self->super_, is_extended, id, dlc, data);
}

ModuleRet TestCan_receive_hp(TestCan* const self, const bool is_extended,
                             const uint32_t id, const uint8_t dlc,
                             const uint8_t* const data) {
  return self->super_.vptr_->receive_hp(&self->super_, is_extended, id, dlc,
                                        data);
}

ModuleRet TestCan_periodic_update(TestCan* const self,
                                  const TickType_t current_tick) {
  return self->super_.vptr_->periodic_update(&self->super_, current_tick);
}

/* constructor ---------------------------------------------------------------*/
void TestCan_ctor(TestCan* self) {
  // construct inherited class and redirect virtual function
  FDCAN_HandleTypeDef can_handle;
  can_handle.Instance = FDCAN1;
  CanTransceiver_ctor(&self->super_, &can_handle);
  static struct CanTransceiverVtbl vtbl = {
      .configure = __TestCan_configure,
      .receive = __TestCan_receive,
      .receive_hp = __TestCan_receive_hp,
      .periodic_update = __TestCan_periodic_update,
  };
  self->super_.vptr_ = &vtbl;
}

/* mock ----------------------------------------------------------------------*/
CanTransceiverMock::CanTransceiverMock() {}

CanTransceiverMock::~CanTransceiverMock() {}

CMOCK_MOCK_FUNCTION(CanTransceiverMock, ModuleRet, __TestCan_configure,
                    (CanTransceiver*));

CMOCK_MOCK_FUNCTION(CanTransceiverMock, ModuleRet, __TestCan_receive,
                    (CanTransceiver*, bool, uint32_t, uint8_t, const uint8_t*));

CMOCK_MOCK_FUNCTION(CanTransceiverMock, ModuleRet, __TestCan_receive_hp,
                    (CanTransceiver*, bool, uint32_t, uint8_t, const uint8_t*));

CMOCK_MOCK_FUNCTION(CanTransceiverMock, ModuleRet, __TestCan_periodic_update,
                    (CanTransceiver*, TickType_t));
