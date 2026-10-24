#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ROUNDING_DST_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ROUNDING_DST_H

//! Validates that the product computed by the MAC DST operation
//! (dest.y = src0.y * src1.y) is truncated toward zero like MUL.
//!
//! Uses the TestVshRoundingMul operands. Expected values are the bit patterns
//! observed on hardware.
//!
//! Validates that DST's y component is:
//! 1. 0x3F801400 for a positive product 0.75 ulp above a representable value.
//! 2. 0xBF801400 for the negated product.
struct TestVshRoundingDst {
  static constexpr const char* Name() { return "VSH Rounding DST"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ROUNDING_DST_H
