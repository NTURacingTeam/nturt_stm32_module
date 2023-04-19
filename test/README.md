# Test for stm32 module

Unit test for stm32 module.

## Test summary

The following shows the lists of unit test for every stm32 module.

Note: Assertion of function parameter checking is not tested.

### button_monitor
- ButtonMonitorInitTest
  - ButtonMonitorInitTest
  - ButtonMonitorAddButton
- ButtonMonitorStartTest
  - ReadStateWileNotStarted
  - StartButtonMonitor
  - AddButtonWhileStarted
- ButtonMonitorReadStateTest
  - ReadStateWithInvalidButtonNumber
  - CleanTransition
  - BouncyTransition
- ButtonMonitorCallbackTest
  - SetCallback
  - ResetCallback
  - RepeatlyCallback

### can_transceiver

- CanTransceiverInitTest
  - CanTransceiverCtor
- CanTransceiverStartTest
  - TransmitWhileNotStarted
  - CanTransceiverStart
- CanTransceiverTransceiveTest
  - PeriodicUpdate
  - Transmit
  - Receive
  - ReceiveHighPriorityMessage
- CanFrameTest
  - AccessTest

### error_handler

- ErrorHandlerInitTest
  - ErrorHandlerCtor
- ErrorHandlerStartTest
  - WriteGetErrorCodeWhileNotStarted
  - ErrorHandlerStart
- ErrorHandlerAccessErrorTest
  - ErrorHandlerWriteError
  - ErrorHandlerGetErrorTest

### led_controller

- LedControllerInitTest
  - LedControllerCtor
  - LedControllerAddLed
- LedControllerStartTest
  - TurnOnOffBlinkWileNotStarted
  - StartLedController
  - AddLedWhileStarted
- LedControllerOnOffBlinkTest
  - TurnOnOffBlinkWithInvalidLedNumber
  - TurnOnLed
  - TurnOnLedWhileOn
  - TurnOffLed
  - TurnOffLedWhileOff
  - TurnOffLedWhileOffAndIsOnBefore
  - RepetedlyTurnOnAndOffLed
  - BlinkLed
  - RepeatedlyBlinkLed
  - ConcurrentlyBlinkLed
  - BlinkLedWhileOn
  - TurnOnLedWhileBlinking
  - TurnOffLedWhileBlinking

## ATTENTION

For those how writing new test for stm32 module, please note:

- Since we are using gcc and g++ multilib to compile the test, remember to choose the correct version of gcc/g++ that match the version of your installed multilib. The default one should do the job.
- Weirdly enought, only `semphr.h` in freertos kernel dose not have `extern "C"` guard like any other header files do. So extra `extern "C"` guard is added for every c header files included in mock and test for extra precaution.
- For some reason, the test cannot properly link to google test library if a test with test fixture is not defined, for example:
  ```cpp
  class FooStartTest : public ::testing::Test {
  protected:
    virtual void SetUp() {
      Foo_ctor(&foo_);
    }

    FreeRTOS_Mock freertos_mock_;

    Foo foo_;
  };

  TEST_F(FoorStartTest, FooStart) {
    EXPECT_CALL(freertos_mock_, xTaskCreateStatic).Times(1);

    EXPECT_EQ(Foo_start(&foo_), ModuleOK);
    EXPECT_EQ(foo_.super_.state_, TaskRunning);
  }
  ```
  UPDATE: May not be a thing anymore.
