#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_TIME_REGISTER_READ_WRITE_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_TIME_REGISTER_READ_WRITE_H

//! Evaluates NV_PTIMER_TIME_0 and NV_PTIMER_TIME_1 read/write isolation and
//! scaling.
//!
//! Validates that:
//! 1. Writing to TIME_0 updates TIME_0 without corrupting TIME_1, and readback
//! matches the written value.
//! 2. Writing to TIME_1 updates TIME_1 without corrupting TIME_0, and readback
//! matches the written value.
struct TestTimeRegisterReadWrite {
  static constexpr const char* Name() { return "Time R/W & 32x Scaling"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_TIME_REGISTER_READ_WRITE_H
