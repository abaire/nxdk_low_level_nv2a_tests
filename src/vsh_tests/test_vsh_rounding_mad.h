#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ROUNDING_MAD_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ROUNDING_MAD_H

//! Validates how the MAC MAD operation rounds its product and its sum.
//!
//! Bit-exact counterpart of the OpRounding_MAD and OpRounding_MADExactProduct
//! depth readback tests in nxdk_pgraph_tests. Each result is compared against
//! single rounding of the exact a * b + c to nearest ("rn") or toward zero
//! ("rz"), against rounding both steps to nearest ("rn_rn") or toward zero
//! ("rz_rz"), and against the "win_rz" adder model applied to RZ(a * b) and c
//! (see RoundingModel). Expected values are the bit patterns observed on
//! hardware, all of which match "win_rz".
//!
//! Observed hardware behavior: MAD is not fused. The product is truncated
//! toward zero and then added to c by the same adder as ADD (see
//! TestVshRoundingAdd).
//!
//! Validates that:
//! 1. a * b - RN(a * b) is -1 ulp for a product 0.75 ulp above a representable
//! value: the product is truncated before the addition.
//! 2. a * 1 + b truncates a sum 0.75 ulp above 1.0 to 1.0: the addition is
//! truncated.
//! 3. A negative inexact product plus a negative addend that leaves the sum
//! inexact gives RZ(RZ(a * b) + c) (0xBF801400), which differs from every
//! other model and from rounding toward -Inf.
//! 4. 1 * 1 - 2^-28 truncates to 0x3F7FFFFF: the addend's bit at the bottom of
//! the window is kept.
//! 5. 1 * 1 - (2^-24 + 2^-29) returns 0x3F7FFFFF: the addend is cut off at the
//! window edge.
struct TestVshRoundingMad {
  static constexpr const char* Name() { return "VSH Rounding MAD"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ROUNDING_MAD_H
