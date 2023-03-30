# Test for stm32 module

Unit test for stm32 module.

## Test summary

The following shows the lists of unit test for every stm32 module.

Note: Assertion of function parameter checking is not tested.

### can_transceiver

### error_handler

### led_controller

- LedControllerInitTest
  - LedControllerCtor
  - LedControllerInitLed
- LedControllerStartTest
  - TurnOnOffBlinkWileNotStarted
  - PartiallyInitializedLed
  - FullyInitializedLed
- LedControllerOnOffTest
  - TurnOnLed
  - TurnOnLedWhileOn
  - TurnOffLed
  - TurnOffLedWhileOff
  - TurnOffLedWhileOffAndIsOnBefore
  - RepetedlyTurnOnAndOffLed
- LedControllerBlinkTest
  - BlinkLed
  - RepeatedlyBlinkLed
  - ConcurrentlyBlinkLed
  - BlinkLedWhileOn
  - TurnOnLedWhileBlinking
  - TurnOffLedWhileBlinking

## ATTENTION

For those how writing new test for stm32 module, please note:

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

    EXPECT_EQ(Foo_start(&foo_), MODULE_OK);
    EXPECT_EQ(foo_.super_.state_, TASK_RUNNING);
  }
  ```
- Weirdly enought, only `semphr.h` in freertos kernel dose not have `extern "C"` guard like any other header files do. So it's manually added in order to test in c++ environment.
