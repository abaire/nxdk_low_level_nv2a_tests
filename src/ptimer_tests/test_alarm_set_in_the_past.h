#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_SET_IN_THE_PAST_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_SET_IN_THE_PAST_H

//! Evaluates NV_PTIMER_ALARM_0 behavior when programmed with a timestamp in the
//! past.
//!
//! Validates that:
//! Setting ALARM_0 to a value less than TIME_0 does not trigger an immediate
//! interrupt (requires future match/crossing).
struct TestAlarmSetInThePast {
  static constexpr const char* Name() { return "Alarm in the Past"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_SET_IN_THE_PAST_H
