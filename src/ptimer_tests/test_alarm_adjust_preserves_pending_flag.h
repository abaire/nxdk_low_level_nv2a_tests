#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_ADJUST_PRESERVES_PENDING_FLAG_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_ADJUST_PRESERVES_PENDING_FLAG_H

//! Evaluates whether writing a new target value to NV_PTIMER_ALARM_0 clears or
//! preserves an already-latched alarm pending flag in NV_PTIMER_INTR_0.
//!
//! Validates that:
//! 1. When NV_PTIMER_INTR_EN_0 is 0 (interrupts masked), crossing an armed
//!    NV_PTIMER_ALARM_0 target latches bit 0 in NV_PTIMER_INTR_0 without
//!    delivering a CPU interrupt.
//! 2. Updating NV_PTIMER_ALARM_0 to a new target in the far future preserves
//!    the pending bit in NV_PTIMER_INTR_0 (the comparator target write does not
//!    clear interrupt status).
//! 3. Unmasking NV_PTIMER_INTR_EN_0 = 1 while the flag remains latched
//!    immediately fires the CPU interrupt callback despite ALARM_0 having been
//!    moved to the future.
struct TestAlarmAdjustPreservesPendingFlag {
  static constexpr const char* Name() { return "Alarm Adjust Preserves Flag"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_ADJUST_PRESERVES_PENDING_FLAG_H
