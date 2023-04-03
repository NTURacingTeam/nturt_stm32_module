// stl include
#include <cstdint>

extern "C" {
// stm32 include
#if defined(STM32G431xx)
#include "stm32g4xx_hal.h"
#elif defined(STM32H723xx)
#include "stm32h7xx_hal.h"
#endif

// stm32_module include
#include "stm32_module/can_transceiver.h"
#include "stm32_module/module_common.h"
}

// gtest include
#include "gtest/gtest.h"

// mock include
#include "mock/can_transceiver_mock.hpp"
#include "mock/freertos_mock.hpp"
#include "mock/hal_can_mock.hpp"
#include "mock/mock_common.hpp"

using ::testing::_;
using ::testing::AllOf;
using ::testing::ArrayWithSize;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Field;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetArrayArgument;
using ::testing::Test;

/* can transceiver initialization test ---------------------------------------*/
TEST(CanTransceiverInitTest, CanTransceiverCtor) {
  CanTransceiver can_transceiver;
  FDCAN_HandleTypeDef can_handle;

  CanTransceiver_ctor(&can_transceiver, &can_handle);

  EXPECT_EQ(can_transceiver.can_handle_, &can_handle);
}

/* can transceiver start test ------------------------------------------------*/
class CanTransceiverStartTest : public Test {
 protected:
  void SetUp() override { TestCan_ctor(&test_can_); }

  TestCan test_can_;

  HAL_CANMock can_mock_;

  CanTransceiverMock can_transceiver_mock_;

  FreertosMock freertos_mock_;
};

TEST_F(CanTransceiverStartTest, TransmitWhileNotStarted) {
  EXPECT_CALL(can_mock_, HAL_FDCAN_AddMessageToTxBuffer).Times(0);
  EXPECT_CALL(can_mock_, HAL_FDCAN_AddMessageToTxFifoQ).Times(0);
  EXPECT_CALL(can_transceiver_mock_, __TestCan_configure).Times(0);

  uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
  EXPECT_EQ(
      CanTransceiver_transmit((CanTransceiver*)&test_can_, false, 0x1, 8, data),
      ModuleError);
}

TEST_F(CanTransceiverStartTest, CanTransceiverStart) {
  EXPECT_CALL(freertos_mock_, xTaskCreateStatic).Times(1);
  EXPECT_CALL(can_transceiver_mock_, __TestCan_configure).Times(1);

  EXPECT_EQ(CanTransceiver_start((CanTransceiver*)&test_can_), ModuleOK);
  EXPECT_EQ(test_can_.super_.super_.state_, TaskRunning);
}

/* can transceiver transceive test -------------------------------------------*/
class CanTransceiverTransceiveTest : public Test {
 protected:
  void SetUp() override {
    EXPECT_CALL(can_mock_, HAL_FDCAN_GetRxFifoFillLevel)
        .WillRepeatedly(Return(0));
    EXPECT_CALL(can_transceiver_mock_, __TestCan_configure)
        .WillOnce(Return(ModuleOK));
    EXPECT_CALL(can_transceiver_mock_, __TestCan_periodic_update)
        .WillRepeatedly(Return(ModuleOK));

    TestCan_ctor(&test_can_);
    CanTransceiver_start((CanTransceiver*)&test_can_);
    // yield for can transceiver to run
    vPortYield();
  }

  void TearDown() override {
    vTaskDelete(test_can_.super_.super_.task_handle_);
  }

  TestCan test_can_;

  HAL_CANMock can_mock_;

  CanTransceiverMock can_transceiver_mock_;
};

TEST_F(CanTransceiverTransceiveTest, PeriodicUpdate) {
  EXPECT_CALL(can_transceiver_mock_, __TestCan_periodic_update)
      .Times(AtLeast(1))
      .WillRepeatedly(Return(ModuleOK));

  // wait some time for periodic update to happen
  vTaskDelay(20);
}

TEST_F(CanTransceiverTransceiveTest, Transmit) {
  uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
  EXPECT_CALL(
      can_mock_,
      HAL_FDCAN_AddMessageToTxFifoQ(
          _,
          AllOf(Field(&FDCAN_TxHeaderTypeDef::Identifier, 0x123),
                Field(&FDCAN_TxHeaderTypeDef::IdType, FDCAN_STANDARD_ID),
                Field(&FDCAN_TxHeaderTypeDef::TxFrameType, FDCAN_DATA_FRAME),
                Field(&FDCAN_TxHeaderTypeDef::DataLength, FDCAN_DLC_BYTES_8)),
          ArrayWithSize(data, 8)))
      .WillOnce(Return(HAL_OK));

  EXPECT_EQ(CanTransceiver_transmit((CanTransceiver*)&test_can_, false, 0x123,
                                    8, data),
            ModuleOK);
}

TEST_F(CanTransceiverTransceiveTest, Receive) {
  FDCAN_RxHeaderTypeDef rx_header = {
      .Identifier = 0x123,
      .IdType = FDCAN_STANDARD_ID,
      .RxFrameType = FDCAN_DATA_FRAME,
      .DataLength = FDCAN_DLC_BYTES_8,
      .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
      .BitRateSwitch = FDCAN_BRS_OFF,
      .FDFormat = FDCAN_CLASSIC_CAN,
      .FilterIndex = 0,
      .IsFilterMatchingFrame = 0,
  };
  const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
  EXPECT_CALL(can_mock_, HAL_FDCAN_GetRxFifoFillLevel)
      .WillOnce(Return(1))
      .RetiresOnSaturation();
  // normal priority message is received at rx fifo0
  EXPECT_CALL(can_mock_, HAL_FDCAN_GetRxMessage(_, FDCAN_RX_FIFO0, _, _))
      .WillOnce(DoAll(SetArgPointee<2>(rx_header),
                      SetArrayArgument<3>(data, data + 8), Return(HAL_OK)));
  EXPECT_CALL(can_transceiver_mock_,
              __TestCan_receive(_, false, 0x123, 8, ArrayWithSize(data, 8)))
      .WillOnce(Return(ModuleOK));

  // wait some time for periodic receive to happen
  vTaskDelay(20);
}

TEST_F(CanTransceiverTransceiveTest, ReceiveHighPriorityMessage) {
  FDCAN_RxHeaderTypeDef rx_header = {
      .Identifier = 0x123,
      .IdType = FDCAN_STANDARD_ID,
      .RxFrameType = FDCAN_DATA_FRAME,
      .DataLength = FDCAN_DLC_BYTES_8,
      .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
      .BitRateSwitch = FDCAN_BRS_OFF,
      .FDFormat = FDCAN_CLASSIC_CAN,
      .FilterIndex = 0,
      .IsFilterMatchingFrame = 0,
  };
  const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
  // high priority message is received at rx fifo1
  EXPECT_CALL(can_mock_, HAL_FDCAN_GetRxMessage(_, FDCAN_RX_FIFO1, _, _))
      .WillOnce(DoAll(SetArgPointee<2>(rx_header),
                      SetArrayArgument<3>(data, data + 8), Return(HAL_OK)));
  EXPECT_CALL(can_transceiver_mock_,
              __TestCan_receive_hp(_, false, 0x123, 8, ArrayWithSize(data, 8)))
      .WillOnce(Return(ModuleOK));

  // simulate interrupt from rx fifo1
  FDCAN_HandleTypeDef fdcan_handle = {.Instance = FDCAN1};
  HAL_FDCAN_RxFifo1Callback(&fdcan_handle, FDCAN_IT_RX_FIFO1_NEW_MESSAGE);
}

int main(int argc, char** argv) { return mock::run_freertos_test(&argc, argv); }
