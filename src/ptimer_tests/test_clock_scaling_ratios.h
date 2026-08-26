#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_CLOCK_SCALING_RATIOS_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_CLOCK_SCALING_RATIOS_H

//! Evaluates NV_PTIMER clock frequency and scaling across various NUMERATOR and
//! DENOMINATOR ratios.
//!
//! Validates that:
//! PTIMER frequency accurately scales proportionally with the NUMERATOR and
//! DENOMINATOR register configuration against real-world elapsed time (QPC).
struct TestClockScalingRatios {
  static constexpr const char* Name() { return "Clock Scaling Ratios"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_CLOCK_SCALING_RATIOS_H
