#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_REGISTER_BITMASKS_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_REGISTER_BITMASKS_H

//! Evaluates register bitmasks and alignment for NV_PTIMER_ALARM_0,
//! NV_PTIMER_TIME_0, and NV_PTIMER_TIME_1.
//!
//! Validates that:
//! 1. ALARM_0 masks out the lower 5 bits (bits [4:0] read back as 0).
//! 2. TIME_0 masks out the lower 5 sub-tick bits (bits [4:0] read back as 0).
//! 3. TIME_1 masks out the upper 3 unused bits (bits [31:29] read back as 0).
struct TestRegisterBitmasks {
  static constexpr const char* Name() { return "Register Bitmasks"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_REGISTER_BITMASKS_H
