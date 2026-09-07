#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_FLAG_IS_SET_WHILE_MASKED_RESET_TIME_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_FLAG_IS_SET_WHILE_MASKED_RESET_TIME_H

//! Evaluates NV_PTIMER_INTR_0_ALARM latching when an alarm is scheduled
//! following an explicit TIME_0 reset while NV_PTIMER_INTR_EN_0 is 0.
//!
//! Validates that:
//! 1. Programming TIME_0 and ALARM_0 with INTR_EN_0 disabled does not trigger
//!    CPU interrupts.
//! 2. As TIME_0 advances past ALARM_0, the alarm pending bit in
//!    NV_PTIMER_INTR_0 is latched and can be detected by polling.
struct TestAlarmFlagIsSetWhileMaskedResetTime {
  static constexpr const char* Name() { return "Alarm Flag Set (Reset Time)"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_FLAG_IS_SET_WHILE_MASKED_RESET_TIME_H
