#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_FLAG_IS_SET_WHILE_MASKED_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_FLAG_IS_SET_WHILE_MASKED_H

//! Evaluates NV_PTIMER_INTR_0_ALARM latching when an alarm is scheduled while
//! NV_PTIMER_INTR_EN_0 is 0 (interrupts masked).
//!
//! Validates that:
//! 1. Scheduling an alarm ahead of free-running TIME_0 while INTR_EN_0 is 0
//!    does not trigger CPU interrupts.
//! 2. As TIME_0 reaches the target, the alarm pending bit in NV_PTIMER_INTR_0
//!    is latched and can be detected by polling.
//! 3. Re-enabling INTR_EN_0 while the pending bit is set immediately asserts
//!    the CPU interrupt.
struct TestAlarmFlagIsSetWhileMasked {
  static constexpr const char* Name() { return "Alarm Flag Set While Masked"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_FLAG_IS_SET_WHILE_MASKED_H
