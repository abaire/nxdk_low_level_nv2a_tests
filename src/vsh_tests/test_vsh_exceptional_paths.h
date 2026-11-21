#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_EXCEPTIONAL_PATHS_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_EXCEPTIONAL_PATHS_H

// Tests of zero meeting infinity or NaN when an operand is read from a vertex
// attribute, and of the fixed function fog distance model built from shader
// operations. Expectations are hardware results, logged with alternative
// models as described in test_vsh_exceptional_mac.h.

//! Validates that MAC operations treat an operand read from a vertex attribute
//! the same way as one read from a constant.
//!
//! Each case runs one shader with A supplied as the v1 attribute (read
//! directly by each instruction) and another with A loaded from c[98] into r0.
//! Both compute A * B, B * A, A * B + C, B * A + C, DP4(A, B), DP3(A, B),
//! DPH(A, B), DPH(B, A), A + B, MIN(A, B) and MAX(A, B) with B = c[96] and
//! C = c[97].
//!
//! Cases:
//! - A = (0, -0, 0, denormal), B = (+inf, +inf, 0x7FC00000, +inf),
//! C = (1.5, 1.5, 1.5, 1.5).
//! - A = (+inf, -inf, 0x7FC00000, 1.5), B = (0, -0, 0, 0), C = 1.5.
//! - A = (1.5, 2, -1.5, 3), B = (2, 0.5, 2, 1), C = 1.5 (finite control).
//!
//! Validates that, on both paths:
//! 1. MUL and the MAD product give +0 for 0 * +-inf, -0 * +inf, 0 * NaN and
//! denormal * +inf in either operand order, so MAD returns C.
//! 2. DP3, DP4 and DPH ignore the products of 0 with +-inf or NaN, while DPH
//! adds src1.w (DPH(A, B) = inf in the first case).
//! 3. ADD gives 0x7FFFFFFF for x + NaN and inf for inf + 0.
//! 4. MIN and MAX follow the sign-magnitude order of TestVshExceptionalMin:
//! MIN(0, +NaN) = 0, MAX(+NaN, 0) = +NaN unaltered, and MIN(denormal, +inf)
//! returns the denormal.
//! 5. Vertex attribute fetch passes NaN payloads, infinities, -0 and
//! denormals through as the constant path does, so the two paths produce
//! identical bits for every result.
struct TestVshExceptionalAttribute {
  static constexpr const char* Name() {
    return "VSH Exceptional Attribute Path";
  }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates the operation chain that models fixed function fog distance for
//! an eye-space position P with w = 0.
//!
//! The shader computes rcp(P.w), p = P * rcp(P.w), d2 = DP3(p, p), rsq(d2)
//! and d = d2 * rsq(d2), with P read from a constant and, in a second run,
//! from the v1 attribute. The intermediates stay in temporary registers.
//!
//! The fixed function results of Fog_eye_space_W in nxdk_pgraph_tests are
//! explained by this chain because 0 * inf = 0 and rsq(inf) = 0: for xyz != 0,
//! p.xyz is infinite (the planar distance p.z is infinite) and the radial
//! distance d is inf * 0 = 0; for xyz = 0, p = 0, d2 = 0 and d = 0 * inf = 0.
//!
//! Validates that, on both paths:
//! 1. rcp(P.w) = +inf for P.w = +0 and -inf for P.w = -0 ("pos_inf" gives +inf
//! for both).
//! 2. p.xyz = +-inf for nonzero xyz with the sign of the product, p is +0 for
//! zero components (including -0 * -inf), and p.w = P.w * inf is +0.
//! 3. d2 = inf whenever any component of xyz is nonzero, and d2 = +0 for
//! xyz = 0.
//! 4. rsq(d2) = +0 for d2 = inf and +inf for d2 = 0.
//! 5. d = +0 in every w = 0 case ("ieee" gives NaN).
//! 6. The reference P = (1, 2, 3, 1) gives rcp = 1, p = P, d2 = 14,
//! rsq(14) = 0x3E88D676 and d = 0x406F774E.
struct TestVshExceptionalFogChain {
  static constexpr const char* Name() { return "VSH Exceptional Fog Chain"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_EXCEPTIONAL_PATHS_H
