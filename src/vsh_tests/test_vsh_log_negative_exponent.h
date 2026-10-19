#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_LOG_NEGATIVE_EXPONENT_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_LOG_NEGATIVE_EXPONENT_H

//! Validates the sign of the exponent returned in LOG.x (LOGP) for inputs
//! between zero and one. Regression test for emulators that store the exponent
//! in an unsigned integer, turning e.g. LOG(0.5).x = -1 into ~4.29e9.
//!
//! The shader emits LOGP(v).x + 128/255 replicated to all color channels, so
//! the probe pixel is gray. A negative exponent (<= -1) yields a sum <= 0 and
//! clamps to black, while an unsigned wrap saturates to white. Hardware
//! produces 0x000000 for every input below 1, 0x808080 for 1.0 and 0xFFFFFF for
//! 2.0.
//!
//! Validates that:
//! 1. LOG(v).x == floor(log2(v)) is negative for v in (0, 1) (0.75, 0.5, 0.25
//! and 0.001 are probed), observed as a red channel of 0.
//! 2. LOG(1).x == 0, observed as red ~= 128 (control).
//! 3. LOG(2).x == 1 saturates red to 255 (control).
struct TestVshLogNegativeExponent {
  static constexpr const char* Name() { return "VSH LOG Negative Exponent"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_LOG_NEGATIVE_EXPONENT_H
