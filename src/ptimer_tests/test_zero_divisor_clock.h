#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ZERO_DIVISOR_CLOCK_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ZERO_DIVISOR_CLOCK_H

//! Evaluates NV_PTIMER stability and behavior when programmed with zero
//! NUMERATOR or DENOMINATOR values.
//!
//! Validates that:
//! Configuring NUMERATOR=0, DENOMINATOR=0, or both does not raise an exception.
struct TestZeroDivisorClock {
  static constexpr const char* Name() { return "Zero-Divisor Clock Handling"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_ZERO_DIVISOR_CLOCK_H
