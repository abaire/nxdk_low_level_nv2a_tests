#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_TIME_REGISTER_ROLLOVER_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_TIME_REGISTER_ROLLOVER_H

//! Evaluates 64-bit carry propagation from NV_PTIMER_TIME_0 to NV_PTIMER_TIME_1
//! upon 32-bit counter overflow.
//!
//! Validates that:
//! When TIME_0 rolls over from 0xFFFFFFFF to 0x00000000, TIME_1 correctly
//! increments by 1.
struct TestTimeRegisterRollover {
  static constexpr const char* Name() { return "Time 64-bit Carry & Rollover"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_TIME_REGISTER_ROLLOVER_H
