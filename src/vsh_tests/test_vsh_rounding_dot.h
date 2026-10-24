#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ROUNDING_DOT_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ROUNDING_DOT_H

//! Validates how the MAC DP3, DPH and DP4 operations round their products and
//! their sum.
//!
//! Bit-exact counterpart of the OpRounding_DP4, OpRounding_DP4SumAB and
//! OpRounding_DP4SumAC depth readback tests in nxdk_pgraph_tests. Each result
//! is compared against single rounding of the exact dot product to nearest
//! ("rn") or toward zero ("rz"), against truncated products summed exactly with
//! the sum truncated ("tp_rz"), against truncating every product and partial
//! sum in x, y, z, w order ("seq_rz"), and against the "win_rz" adder model
//! (see RoundingModel). Expected values are the bit patterns observed on
//! hardware, all of which match "win_rz".
//!
//! Validates that DP3, DPH and DP4 produce:
//! 1. RZ(a * b) - RN(a * b) for (a, RN(a * b)) . (b, -1) with a product 0.75
//! ulp above a representable value: each product is truncated before it is
//! accumulated.
//! 2. The exact a + b - RN(a + b) for (a, b, RN(a + b)) . (1, 1, -1) with a
//! sum 0.75 ulp above 1.0: the partial sum a + b is not rounded.
//! 3. RZ(a + b) for (a, b) . (1, 1) with sums 0.75 ulp beyond +1.0 and -1.0:
//! the final sum is truncated toward zero.
//!
//! Validates that:
//! 4. DPH of (a, b, 0, 0) . (1, 1, 0, -RN(a + b)) gives the exact
//! a + b - RN(a + b): src1.w joins the same unrounded accumulation.
//! 5. With NaN in both w components of the Rugby 2005 product case, DP3
//! ignores w and returns RZ(a * b) - RN(a * b) = 0xB6800000, while DPH and DP4
//! return the canonical NaN 0x7FFFFFFF.
struct TestVshRoundingDot {
  static constexpr const char* Name() { return "VSH Rounding DP3 DPH DP4"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates the width of the DP3, DPH and DP4 accumulator.
//!
//! Uses the same models as TestVshRoundingDot. Expected values are the bit
//! patterns observed on hardware, all of which match "win_rz".
//!
//! Observed hardware behavior: every truncated product is aligned to the
//! largest one in a 29-bit window with no sticky bit, the same window as ADD
//! and MAD (see TestVshRoundingAdd), so bits below 2^-28 relative to the
//! largest product's leading bit are discarded regardless of the order of the
//! terms.
//!
//! Validates that DP3, DPH and DP4 produce:
//! 1. 2^-28 for 1 + 2^-28 - 1: a bit at the bottom of the window is kept.
//! 2. 0x3F7FFFFF for (1, -(2^-24 + 2^-29)) . (1, 1): the second term is cut
//! off at the window edge.
//! 3. +0 for (2^32, -2^32, 2^-32) . (1, 1, 1): the window is anchored to the
//! largest term rather than to the running sum, so the small term is lost even
//! though the large terms cancel first.
struct TestVshRoundingDotAccumulation {
  static constexpr const char* Name() {
    return "VSH Rounding DP3 DPH DP4 Accumulation";
  }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ROUNDING_DOT_H
