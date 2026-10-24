#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ROUNDING_MUL_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ROUNDING_MUL_H

//! Validates that the MAC MUL operation truncates its product toward zero.
//!
//! Bit-exact counterpart of the OpRounding_MUL depth readback test in
//! nxdk_pgraph_tests. Each result is compared against IEEE
//! round-to-nearest-even ("rn") and truncation ("rz") of the exact product.
//! Expected values are the bit patterns observed on hardware.
//!
//! Validates that:
//! 1. A positive product 0.75 ulp above a representable value is truncated
//! (0x3F801400 rather than the round-to-nearest 0x3F801401).
//! 2. The negated product is truncated toward zero, not toward -Inf
//! (0xBF801400).
struct TestVshRoundingMul {
  static constexpr const char* Name() { return "VSH Rounding MUL"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ROUNDING_MUL_H
