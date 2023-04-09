#ifndef CAN_TRANSCEIVER_MOCK_HPP
#define CAN_TRANSCEIVER_MOCK_HPP

extern "C" {
// stm32_module include
#include "stm32_module/can_transceiver.h"
#include "stm32_module/module_common.h"
}

// gtest include
#include "cmock/cmock.h"

/* class inherited from CanTransceiver for testing ---------------------------*/
typedef struct test_can {
  CanTransceiver super_;
} TestCan;

/* constructor ---------------------------------------------------------------*/
void TestCan_ctor(TestCan* self, CanHandle* const can_handle);

/* member function -----------------------------------------------------------*/
ModuleRet TestCan_configure(TestCan* const self);

ModuleRet TestCan_receive(TestCan* const self, const bool is_extended,
                          const uint32_t id, const uint8_t dlc,
                          const uint8_t* const data);

ModuleRet TestCan_receive_hp(TestCan* const self, const bool is_extended,
                             const uint32_t id, const uint8_t dlc,
                             const uint8_t* const data);

ModuleRet TestCan_periodic_update(TestCan* const self,
                                  const TickType_t current_tick);

/* virtual function declaration ----------------------------------------------*/
ModuleRet __TestCan_configure(CanTransceiver* self);

ModuleRet __TestCan_receive(CanTransceiver* self, bool is_extended, uint32_t id,
                            uint8_t dlc, const uint8_t* data);

ModuleRet __TestCan_receive_hp(CanTransceiver* self, bool is_extended,
                               uint32_t id, uint8_t dlc, const uint8_t* data);

ModuleRet __TestCan_periodic_update(CanTransceiver* self, TickType_t period);

/* mock ----------------------------------------------------------------------*/
class CanTransceiverMock : public CMockMocker<CanTransceiverMock> {
 public:
  CanTransceiverMock();

  ~CanTransceiverMock();

  CMOCK_MOCK_METHOD(ModuleRet, __TestCan_configure, (CanTransceiver*));

  CMOCK_MOCK_METHOD(ModuleRet, __TestCan_receive,
                    (CanTransceiver*, bool, uint32_t, uint8_t, const uint8_t*));

  CMOCK_MOCK_METHOD(ModuleRet, __TestCan_receive_hp,
                    (CanTransceiver*, bool, uint32_t, uint8_t, const uint8_t*));

  CMOCK_MOCK_METHOD(ModuleRet, __TestCan_periodic_update,
                    (CanTransceiver*, TickType_t));
};

#endif  // CAN_TRANSCEIVER_MOCK_HPP
