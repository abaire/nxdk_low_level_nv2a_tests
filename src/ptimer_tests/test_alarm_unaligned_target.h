#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_UNALIGNED_TARGET_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_UNALIGNED_TARGET_H

//! Evaluates NV_PTIMER_ALARM_0 comparator evaluation when given an unaligned
//! target.
//!
//! Validates that:
//! When writing an unaligned value to ALARM_0 where the unmasked bits would
//! exceed TIME_0 but the masked bits are <= TIME_0, the comparator masks the
//! value and does not fire as an immediate future alarm.
struct TestAlarmUnalignedTarget {
  static constexpr const char* Name() { return "Alarm Unaligned Target"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ALARM_UNALIGNED_TARGET_H
