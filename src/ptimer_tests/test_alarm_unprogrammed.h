#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_UNPROGRAMMED_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_UNPROGRAMMED_H

//! Evaluates the unprogrammed boot/initial state of NV_PTIMER_ALARM_0 and
//! verifies whether an unconfigured alarm fires when NV_PTIMER_TIME_0 crosses
//! it.
//!
//! Validates that:
//! 1. Prior to pb_init(), the untouched boot values of NV_PTIMER_ALARM_0,
//!    NV_PTIMER_INTR_EN_0, NV_PTIMER_INTR_0, NV_PTIMER_NUMERATOR, and
//!    NV_PTIMER_DENOMINATOR are captured directly at main() entry.
//! 2. Prior to pb_init(), before NV_PTIMER_ALARM_0 is ever written by software,
//!    NV_PTIMER_INTR_0 bit 0 latches as NV_PTIMER_TIME_0 crosses the boot alarm
//!    target.
//! 3. After pb_init() installs the GPU ISR, unmasking NV_PTIMER_INTR_EN_0
//! delivers
//!    CPU interrupts when NV_PTIMER_ALARM_0 is restored to the boot value.
struct TestAlarmUnprogrammed {
  static constexpr const char* Name() { return "Alarm Unprogrammed at Boot"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_UNPROGRAMMED_H
