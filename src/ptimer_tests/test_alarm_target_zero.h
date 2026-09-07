#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_TARGET_ZERO_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_TARGET_ZERO_H

//! Evaluates NV_PTIMER_ALARM_0 behavior when programmed with target value 0.
//!
//! Validates that:
//! 1. Programming NV_PTIMER_ALARM_0 with 0 is treated as a valid alarm target
//!    rather than a disabled/disarmed sentinel.
//! 2. As free-running NV_PTIMER_TIME_0 rolls over from 0xFF000000 across 0,
//!    the hardware matches the alarm target and latches NV_PTIMER_INTR_0 bit 0.
//! 3. With NV_PTIMER_INTR_EN_0 enabled, the CPU interrupt callback fires upon
//!    reaching 0.
struct TestAlarmTargetZero {
  static constexpr const char* Name() { return "Alarm Target Zero"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_TARGET_ZERO_H
