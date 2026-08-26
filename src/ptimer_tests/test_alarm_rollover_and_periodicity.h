#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_ROLLOVER_AND_PERIODICITY_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_ROLLOVER_AND_PERIODICITY_H

//! Evaluates NV_PTIMER_ALARM_0 continuous periodic firing across 32-bit counter
//! rollover.
//!
//! Validates that:
//! When ALARM_0 is left configured across a full 2^32 counter wrap (~4.295s),
//! needing to be rearmed.
struct TestAlarmRolloverAndPeriodicity {
  static constexpr const char* Name() { return "Alarm Rollover & Periodicity"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_ROLLOVER_AND_PERIODICITY_H
