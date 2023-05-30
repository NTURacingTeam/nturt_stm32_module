// stl include
#include <cstdint>

extern "C" {
// stm32 include
#include "stm32_module/stm32_hal.h"

// stm32_module include
#include "stm32_module/stm32_module.h"
}

// gtest include
#include "gtest/gtest.h"

// mock include
#include "mock/mock.hpp"
using ::testing::_;
using ::testing::AllOf;
using ::testing::ArrayWithSize;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Field;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetArrayArgument;
using ::testing::Test;
using ::testing::WithArg;

/* test parameters -----------------------------------------------------------*/
#define NUM_CAN_TRANSCIEVER 5

/* other variables -----------------------------------------------------------*/
extern bool is_first_can_transceiver;

/* can transceiver initialization test ---------------------------------------*/
TEST(CanTransceiverInitTest, CanTransceiverCtor) {
  CanTransceiver can_transceiver;
  CanHandle can_handle;

  CanTransceiver_ctor(&can_transceiver, &can_handle);

  EXPECT_EQ(can_transceiver.can_handle_, &can_handle);
}

/* can transceiver start test ------------------------------------------------*/
class CanTransceiverStartTest : public Test {
 protected:
  void SetUp() override {
    // reset can transceiver list
    is_first_can_transceiver = true;
    TestCan_ctor(&test_can_, &can_handle_);
  }

  TestCan test_can_;

  CanHandle can_handle_;

  HAL_CANMock can_mock_;

  CanTransceiverMock can_transceiver_mock_;

  FreertosMock freertos_mock_;
};

TEST_F(CanTransceiverStartTest, TransmitWhileNotStarted) {
#if defined(HAL_CAN_MODULE_ENABLED)
  EXPECT_CALL(can_mock_, HAL_CAN_AddTxMessage).Times(0);
#elif defined(HAL_FDCAN_MODULE_ENABLED)
  EXPECT_CALL(can_mock_, HAL_FDCAN_AddMessageToTxBuffer).Times(0);
  EXPECT_CALL(can_mock_, HAL_FDCAN_AddMessageToTxFifoQ).Times(0);
#endif
  EXPECT_CALL(can_transceiver_mock_, __TestCan_configure).Times(0);

  uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
  EXPECT_EQ(
      CanTransceiver_transmit((CanTransceiver*)&test_can_, false, 0x1, 8, data),
      ModuleError);
}

TEST_F(CanTransceiverStartTest, CanTransceiverStart) {
  EXPECT_CALL(freertos_mock_, xTaskCreateStatic)
      .WillOnce(
          WithArg<6>(Invoke([](StaticTask_t* t) { return (TaskHandle_t)t; })));
  EXPECT_CALL(can_transceiver_mock_, __TestCan_configure).Times(1);

  EXPECT_EQ(CanTransceiver_start((CanTransceiver*)&test_can_), ModuleOK);
  EXPECT_EQ(test_can_.super_.super_.state_, TaskRunning);
}

/* can transceiver transceive test -------------------------------------------*/
class CanTransceiverTransceiveTest : public Test {
 protected:
  void SetUp() override {
#if defined(HAL_CAN_MODULE_ENABLED)
    EXPECT_CALL(can_mock_, HAL_CAN_GetRxFifoFillLevel)
        .WillRepeatedly(Return(0));
#elif defined(HAL_FDCAN_MODULE_ENABLED)
    EXPECT_CALL(can_mock_, HAL_FDCAN_GetRxFifoFillLevel)
        .WillRepeatedly(Return(0));
#endif
    EXPECT_CALL(can_transceiver_mock_, __TestCan_configure).Times(1);
    EXPECT_CALL(can_transceiver_mock_, __TestCan_periodic_update)
        .Times(AtLeast(1));

    // reset can transceiver list
    is_first_can_transceiver = true;
    TestCan_ctor(&test_can_, &can_handle_);
    CanTransceiver_start((CanTransceiver*)&test_can_);
    // yield for can transceiver to run
    vPortYield();
  }

  void TearDown() override { Task_delete((Task*)&test_can_); }

  TestCan test_can_;

  CanHandle can_handle_;

  HAL_CANMock can_mock_;

  CanTransceiverMock can_transceiver_mock_;
};

TEST_F(CanTransceiverTransceiveTest, PeriodicUpdate) {
  EXPECT_CALL(can_transceiver_mock_, __TestCan_periodic_update)
      .Times(AtLeast(1));

  // wait some time for periodic update to happen
  vTaskDelay(20);
}

TEST_F(CanTransceiverTransceiveTest, Transmit) {
  uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
#if defined(HAL_CAN_MODULE_ENABLED)
  EXPECT_CALL(
      can_mock_,
      HAL_CAN_AddTxMessage(_,
                           AllOf(Field(&CAN_TxHeaderTypeDef::StdId, 0x123),
                                 Field(&CAN_TxHeaderTypeDef::IDE, CAN_ID_STD),
                                 Field(&CAN_TxHeaderTypeDef::RTR, CAN_RTR_DATA),
                                 Field(&CAN_TxHeaderTypeDef::DLC, 8)),
                           ArrayWithSize(data, 8), _))
      .WillOnce(Return(HAL_OK));
#elif defined(HAL_FDCAN_MODULE_ENABLED)
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
#endif

  EXPECT_EQ(CanTransceiver_transmit((CanTransceiver*)&test_can_, false, 0x123,
                                    8, data),
            ModuleOK);
}

TEST_F(CanTransceiverTransceiveTest, Receive) {
  const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
#if defined(HAL_CAN_MODULE_ENABLED)
  CAN_RxHeaderTypeDef rx_header = {
      .StdId = 0x123,
      .ExtId = 0,
      .IDE = CAN_ID_STD,
      .RTR = CAN_RTR_DATA,
      .DLC = 8,
      .Timestamp = 0,
      .FilterMatchIndex = 0,
  };
  EXPECT_CALL(can_mock_, HAL_CAN_GetRxFifoFillLevel)
      .WillOnce(Return(1))
      .RetiresOnSaturation();
  // normal priority message is received at rx fifo0
  EXPECT_CALL(can_mock_, HAL_CAN_GetRxMessage(_, CAN_RX_FIFO0, _, _))
      .WillOnce(DoAll(SetArgPointee<2>(rx_header),
                      SetArrayArgument<3>(data, data + 8), Return(HAL_OK)))
      .RetiresOnSaturation();
#elif defined(HAL_FDCAN_MODULE_ENABLED)
  FDCAN_RxHeaderTypeDef rx_header = {
      .Identifier = 0x123,
      .IdType = FDCAN_STANDARD_ID,
      .RxFrameType = FDCAN_DATA_FRAME,
      .DataLength = FDCAN_DLC_BYTES_8,
      .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
      .BitRateSwitch = FDCAN_BRS_OFF,
      .FDFormat = FDCAN_CLASSIC_CAN,
      .RxTimestamp = 0,
      .FilterIndex = 0,
      .IsFilterMatchingFrame = 0,
  };
  EXPECT_CALL(can_mock_, HAL_FDCAN_GetRxFifoFillLevel)
      .WillOnce(Return(1))
      .RetiresOnSaturation();
  // normal priority message is received at rx fifo0
  EXPECT_CALL(can_mock_, HAL_FDCAN_GetRxMessage(_, FDCAN_RX_FIFO0, _, _))
      .WillOnce(DoAll(SetArgPointee<2>(rx_header),
                      SetArrayArgument<3>(data, data + 8), Return(HAL_OK)));
#endif
  EXPECT_CALL(can_transceiver_mock_,
              __TestCan_receive(_, false, 0x123, 8, ArrayWithSize(data, 8)))
      .Times(1);

  // wait some time for periodic receive to happen
  vTaskDelay(20);
}

TEST_F(CanTransceiverTransceiveTest, ReceiveHighPriorityMessage) {
  const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
  EXPECT_CALL(can_transceiver_mock_,
              __TestCan_receive_hp(_, false, 0x123, 8, ArrayWithSize(data, 8)))
      .Times(1);
#if defined(HAL_CAN_MODULE_ENABLED)
  CAN_RxHeaderTypeDef rx_header = {
      .StdId = 0x123,
      .ExtId = 0,
      .IDE = CAN_ID_STD,
      .RTR = CAN_RTR_DATA,
      .DLC = 8,
      .Timestamp = 0,
      .FilterMatchIndex = 0,
  };
  // high priority message is received at rx fifo1
  EXPECT_CALL(can_mock_, HAL_CAN_GetRxMessage(_, CAN_RX_FIFO1, _, _))
      .WillOnce(DoAll(SetArgPointee<2>(rx_header),
                      SetArrayArgument<3>(data, data + 8), Return(HAL_OK)));
  // simulate interrupt from rx fifo1
  HAL_CAN_RxFifo1MsgPendingCallback(&can_handle_);
#elif defined(HAL_FDCAN_MODULE_ENABLED)
  FDCAN_RxHeaderTypeDef rx_header = {
      .Identifier = 0x123,
      .IdType = FDCAN_STANDARD_ID,
      .RxFrameType = FDCAN_DATA_FRAME,
      .DataLength = FDCAN_DLC_BYTES_8,
      .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
      .BitRateSwitch = FDCAN_BRS_OFF,
      .FDFormat = FDCAN_CLASSIC_CAN,
      .RxTimestamp = 0,
      .FilterIndex = 0,
      .IsFilterMatchingFrame = 0,
  };
  // high priority message is received at rx fifo1
  EXPECT_CALL(can_mock_, HAL_FDCAN_GetRxMessage(_, FDCAN_RX_FIFO1, _, _))
      .WillOnce(DoAll(SetArgPointee<2>(rx_header),
                      SetArrayArgument<3>(data, data + 8), Return(HAL_OK)));
  // simulate interrupt from rx fifo1
  HAL_FDCAN_RxFifo1Callback(&can_handle_, FDCAN_IT_RX_FIFO1_NEW_MESSAGE);
#endif
}

class MultiCanTransceiver : public Test {
 protected:
  void SetUp() override {
#if defined(HAL_CAN_MODULE_ENABLED)
    EXPECT_CALL(can_mock_, HAL_CAN_GetRxFifoFillLevel)
        .WillRepeatedly(Return(0));
#elif defined(HAL_FDCAN_MODULE_ENABLED)
    EXPECT_CALL(can_mock_, HAL_FDCAN_GetRxFifoFillLevel)
        .WillRepeatedly(Return(0));
#endif
    EXPECT_CALL(can_transceiver_mock_, __TestCan_configure)
        .Times(NUM_CAN_TRANSCIEVER);
    EXPECT_CALL(can_transceiver_mock_, __TestCan_periodic_update)
        .Times(AtLeast(1));

    // reset can transceiver list
    is_first_can_transceiver = true;
    for (int i = 0; i < NUM_CAN_TRANSCIEVER; i++) {
      TestCan_ctor(&test_can_[i], &can_handle_[i]);
      CanTransceiver_start((CanTransceiver*)&test_can_[i]);
    }
    // yield for can transceiver to run
    vPortYield();
  }

  void TearDown() override {
    for (int i = 0; i < NUM_CAN_TRANSCIEVER; i++) {
      Task_delete((Task*)&test_can_[i]);
    }
  }

  TestCan test_can_[NUM_CAN_TRANSCIEVER];

  CanHandle can_handle_[NUM_CAN_TRANSCIEVER];

  HAL_CANMock can_mock_;

  CanTransceiverMock can_transceiver_mock_;
};

TEST_F(MultiCanTransceiver, Transmit) {
  uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
  for (int i = 0; i < NUM_CAN_TRANSCIEVER; i++) {
#if defined(HAL_CAN_MODULE_ENABLED)
    EXPECT_CALL(can_mock_,
                HAL_CAN_AddTxMessage(
                    &can_handle_[i],
                    AllOf(Field(&CAN_TxHeaderTypeDef::StdId, 0x123),
                          Field(&CAN_TxHeaderTypeDef::IDE, CAN_ID_STD),
                          Field(&CAN_TxHeaderTypeDef::RTR, CAN_RTR_DATA),
                          Field(&CAN_TxHeaderTypeDef::DLC, 8)),
                    ArrayWithSize(data, 8), _))
        .WillOnce(Return(HAL_OK));
#elif defined(HAL_FDCAN_MODULE_ENABLED)
    EXPECT_CALL(
        can_mock_,
        HAL_FDCAN_AddMessageToTxFifoQ(
            &can_handle_[i],
            AllOf(Field(&FDCAN_TxHeaderTypeDef::Identifier, 0x123),
                  Field(&FDCAN_TxHeaderTypeDef::IdType, FDCAN_STANDARD_ID),
                  Field(&FDCAN_TxHeaderTypeDef::TxFrameType, FDCAN_DATA_FRAME),
                  Field(&FDCAN_TxHeaderTypeDef::DataLength, FDCAN_DLC_BYTES_8)),
            ArrayWithSize(data, 8)))
        .WillOnce(Return(HAL_OK));
#endif
  }
  for (int i = 0; i < NUM_CAN_TRANSCIEVER; i++) {
    EXPECT_EQ(CanTransceiver_transmit((CanTransceiver*)&test_can_[i], false,
                                      0x123, 8, data),
              ModuleOK);
  }
}

TEST_F(MultiCanTransceiver, Receive) {
  const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
#if defined(HAL_CAN_MODULE_ENABLED)
  CAN_RxHeaderTypeDef rx_header = {
      .StdId = 0x123,
      .ExtId = 0,
      .IDE = CAN_ID_STD,
      .RTR = CAN_RTR_DATA,
      .DLC = 8,
      .Timestamp = 0,
      .FilterMatchIndex = 0,
  };
#elif defined(HAL_FDCAN_MODULE_ENABLED)
  FDCAN_RxHeaderTypeDef rx_header = {
      .Identifier = 0x123,
      .IdType = FDCAN_STANDARD_ID,
      .RxFrameType = FDCAN_DATA_FRAME,
      .DataLength = FDCAN_DLC_BYTES_8,
      .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
      .BitRateSwitch = FDCAN_BRS_OFF,
      .FDFormat = FDCAN_CLASSIC_CAN,
      .RxTimestamp = 0,
      .FilterIndex = 0,
      .IsFilterMatchingFrame = 0,
  };
#endif
  for (int i = 0; i < NUM_CAN_TRANSCIEVER; i++) {
#if defined(HAL_CAN_MODULE_ENABLED)
    EXPECT_CALL(can_mock_, HAL_CAN_GetRxFifoFillLevel(&can_handle_[i], _))
        .WillOnce(Return(1))
        .RetiresOnSaturation();
    // normal priority message is received at rx fifo0
    EXPECT_CALL(can_mock_,
                HAL_CAN_GetRxMessage(&can_handle_[i], CAN_RX_FIFO0, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(rx_header),
                        SetArrayArgument<3>(data, data + 8), Return(HAL_OK)));
#elif defined(HAL_FDCAN_MODULE_ENABLED)
    EXPECT_CALL(can_mock_, HAL_FDCAN_GetRxFifoFillLevel(&can_handle_[i], _))
        .WillOnce(Return(1))
        .RetiresOnSaturation();
    // normal priority message is received at rx fifo0
    EXPECT_CALL(can_mock_,
                HAL_FDCAN_GetRxMessage(&can_handle_[i], FDCAN_RX_FIFO0, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(rx_header),
                        SetArrayArgument<3>(data, data + 8), Return(HAL_OK)));
#endif
    EXPECT_CALL(can_transceiver_mock_,
                __TestCan_receive((CanTransceiver*)&test_can_[i], false, 0x123,
                                  8, ArrayWithSize(data, 8)))
        .Times(1);
  }

  // wait some time for periodic receive to happen
  vTaskDelay(20);
}

TEST_F(MultiCanTransceiver, ReceiveHighPriorityMessage) {
  const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
#if defined(HAL_CAN_MODULE_ENABLED)
  CAN_RxHeaderTypeDef rx_header = {
      .StdId = 0x123,
      .ExtId = 0,
      .IDE = CAN_ID_STD,
      .RTR = CAN_RTR_DATA,
      .DLC = 8,
      .Timestamp = 0,
      .FilterMatchIndex = 0,
  };
#elif defined(HAL_FDCAN_MODULE_ENABLED)
  FDCAN_RxHeaderTypeDef rx_header = {
      .Identifier = 0x123,
      .IdType = FDCAN_STANDARD_ID,
      .RxFrameType = FDCAN_DATA_FRAME,
      .DataLength = FDCAN_DLC_BYTES_8,
      .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
      .BitRateSwitch = FDCAN_BRS_OFF,
      .FDFormat = FDCAN_CLASSIC_CAN,
      .RxTimestamp = 0,
      .FilterIndex = 0,
      .IsFilterMatchingFrame = 0,
  };
#endif
  for (int i = 0; i < NUM_CAN_TRANSCIEVER; i++) {
    // high priority message is received at rx fifo1
#if defined(HAL_CAN_MODULE_ENABLED)
    EXPECT_CALL(can_mock_,
                HAL_CAN_GetRxMessage(&can_handle_[i], CAN_RX_FIFO1, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(rx_header),
                        SetArrayArgument<3>(data, data + 8), Return(HAL_OK)));
#elif defined(HAL_FDCAN_MODULE_ENABLED)
    EXPECT_CALL(can_mock_,
                HAL_FDCAN_GetRxMessage(&can_handle_[i], FDCAN_RX_FIFO1, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(rx_header),
                        SetArrayArgument<3>(data, data + 8), Return(HAL_OK)));
#endif
    EXPECT_CALL(can_transceiver_mock_,
                __TestCan_receive_hp((CanTransceiver*)&test_can_[i], false,
                                     0x123, 8, ArrayWithSize(data, 8)))
        .Times(1);
  }

  // simulate interrupt from rx fifo1
  for (int i = 0; i < NUM_CAN_TRANSCIEVER; i++) {
#if defined(HAL_CAN_MODULE_ENABLED)
    HAL_CAN_RxFifo1MsgPendingCallback(&can_handle_[i]);
#elif defined(HAL_FDCAN_MODULE_ENABLED)
    HAL_FDCAN_RxFifo1Callback(&can_handle_[i], FDCAN_IT_RX_FIFO1_NEW_MESSAGE);
#endif
  }
}

int main(int argc, char** argv) { return mock::run_freertos_test(&argc, argv); }
