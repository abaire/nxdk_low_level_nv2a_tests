#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ROUNDING_ADD_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ROUNDING_ADD_H

//! Validates how the MAC ADD operation aligns and rounds its operands.
//!
//! Bit-exact counterpart of the OpRounding_ADD depth readback test in
//! nxdk_pgraph_tests. Each result is compared against IEEE
//! round-to-nearest-even ("rn"), truncation ("rz") of the exact sum, and the
//! "win_rz" adder model (see RoundingModel). Expected values are the bit
//! patterns observed on hardware, all of which match "win_rz".
//!
//! Observed hardware behavior: the smaller operand is aligned to the larger one
//! in a 29-bit window with no sticky bit, discarding its bits below 2^-28
//! relative to the larger operand's leading bit, and the sum is truncated
//! toward zero.
//!
//! Validates that:
//! 1. 1 + 0.75 ulp truncates to 1.0 (round-to-nearest gives 0x3F800001).
//! 2. -1 - 0.75 ulp truncates toward zero to -1.0, not toward -Inf.
//! 3. 1 - 2^-28 truncates to 0x3F7FFFFF: a bit at the bottom of the window is
//! kept.
//! 4. 1 - (2^-24 + 2^-29) returns 0x3F7FFFFF: the operand's 2^-29 bit is cut
//! off at the window edge while its 2^-24 bit is kept (truncating the exact
//! difference would give 0x3F7FFFFE).
struct TestVshRoundingAdd {
  static constexpr const char* Name() { return "VSH Rounding ADD"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ROUNDING_ADD_H
