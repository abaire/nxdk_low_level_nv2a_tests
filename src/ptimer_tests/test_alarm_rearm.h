#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_REARM_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_REARM_H

//! Evaluates explicit NV_PTIMER_ALARM_0 re-arming and firing in the future.
//!
//! Validates that:
//! Reprogramming ALARM_0 to a future timestamp while the timer is running
//! correctly triggers an interrupt when TIME_0 reaches the target.
struct TestAlarmRearm {
  static constexpr const char* Name() { return "Alarm Explicit Re-arm"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_REARM_H
