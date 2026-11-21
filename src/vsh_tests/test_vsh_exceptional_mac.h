#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_EXCEPTIONAL_MAC_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_EXCEPTIONAL_MAC_H

// Tests of MAC operations on signed zeros, infinities, NaNs and values at the
// edges of the float32 range. Results are read back bit-exactly from constant
// registers.
//
// Every expectation is the result observed on hardware.
//
// Conventions established by other tests and relied on here:
// - Products and sums are truncated toward zero (see TestVshRoundingMul and
// TestVshRoundingAdd), and an exact cancellation gives +0.
// - DP4 and DPH return 0x7FFFFFFF when an operand is NaN, and DP3 ignores w
// even when it holds NaN or Inf (TestVshRoundingDot).
// - Arithmetic reads a denormal operand as zero (TestVshLogDenormal). MOV,
// MIN, MAX, SLT and SGE do not (see below).
//
// Unless stated otherwise the first operand of every two-operand case is
// copied through r0 with MOV, so TestVshExceptionalMov is a precondition for
// the other tests.

//! Validates how MOV treats special values, including negation, swizzles and
//! both input paths.
//!
//! The same four vectors are loaded into c[96] and supplied as the v1
//! attribute. Each is copied directly to a constant, negated, swizzled
//! (wzyx), negated and swizzled (yxwz), copied through r0 (plain and negated)
//! and copied from v1 (plain and negated). The vectors hold NaNs
//! (0x7FFFFFFF, 0x7FC00000, 0xFFC00000, 0x7FA5A5A5); +0, -0, +inf, -inf;
//! FLT_MAX, 2^-126, the denormals 0x00400000 and 0x80400000; and 1.5, -1.5,
//! the signaling NaN 0x7F800001 and 0xFFFFFFFF.
//!
//! Validates that every variant produces, per component:
//! 1. The input bits unaltered, including every NaN payload, -0 and
//! denormals, which are not flushed.
//! 2. The input with only its sign bit flipped when negated, including NaNs,
//! zeros and denormals.
//! 3. The swizzled components, unaltered.
//! 4. The same result through r0 as directly, and the same result from v1 as
//! from c[96].
struct TestVshExceptionalMov {
  static constexpr const char* Name() { return "VSH Exceptional MOV"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates MUL with zero, infinity and NaN operands, and at the edges of the
//! float32 range.
//!
//! Validates that MUL produces:
//! 1. +0 for 0 * +-inf and +-inf * 0 in either operand order, whatever the
//! signs.
//! 2. +0 for 0 * NaN and NaN * 0 for every NaN encoding and sign.
//! 3. +0 for +-denormal * +inf, since the denormal is read as zero.
//! 4. +0 for -0 * 1.5 and +0 * -1.5: a product with a zero operand is always
//! +0.
//! 5. The ordinary results for +-1.5 * +inf, inf * inf, FLT_MAX * 0 and
//! 2^-126 * 1.5.
//! 6. 0x7FFFFFFF for NaN * x for every NaN encoding.
//! 7. +-FLT_MAX for FLT_MAX * +-2: overflow truncates instead of producing
//! infinity.
//! 8. +-0 for +-2^-126 * 0.5: a result below 2^-126 is flushed to a zero that
//! keeps the sign of the product, unlike item 4.
struct TestVshExceptionalMul {
  static constexpr const char* Name() { return "VSH Exceptional MUL"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates ADD with infinite, NaN and signed zero operands.
//!
//! Validates that ADD produces:
//! 1. 0x7FFFFFFF for +inf + -inf in either order.
//! 2. inf for inf + inf of the same sign and for inf + any finite value.
//! 3. 0x7FFFFFFF for NaN + x for every NaN encoding.
//! 4. +0 for every sum of zeros, including -0 + -0.
//! 5. +-FLT_MAX for +-(FLT_MAX + FLT_MAX), the truncated result.
//! 6. +0 for denormal + denormal, and 2^-126 for 2^-126 + -denormal: the
//! denormals are read as zero.
//! 7. +0 for 1.5 * 2^-126 - 2^-126, flushing the denormal result.
struct TestVshExceptionalAdd {
  static constexpr const char* Name() { return "VSH Exceptional ADD"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates MAD when the product or the addition stage involves zero,
//! infinity or NaN.
//!
//! Validates that MAD produces:
//! 1. c for 0 * +-inf + c and 0 * NaN + c: the product is +0.
//! 2. +0 for -0 * +inf + -0 and +0 * +inf + -0, as for a +0 product added to
//! -0.
//! 3. -inf for +inf * 0 + -inf, and +-inf for a * b + +-inf with finite a, b.
//! 4. 0x7FFFFFFF when the addition stage computes inf - inf (1 * +inf + -inf,
//! +inf * +inf + -inf, -inf * 1.5 + +inf) or receives a NaN addend.
//! 5. c for denormal * +inf + c, since the denormal is read as zero.
//! 6. +0 for FLT_MAX * 2 - FLT_MAX: the product is truncated to FLT_MAX before
//! the addition.
//! 7. FLT_MAX for FLT_MAX * 1 + FLT_MAX.
struct TestVshExceptionalMad {
  static constexpr const char* Name() { return "VSH Exceptional MAD"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates DP3, DPH and DP4 when a zero component meets an infinite or NaN
//! component, and when infinite products cancel.
//!
//! Validates that DP3, DPH and DP4 produce:
//! 1. The sum of the remaining products when one component pair is 0 and
//! +-inf or NaN, in either operand, including -0 * +inf and denormal * +inf.
//! 2. For (1, 1, 1, 0) . (1, 1, 1, inf): DP3 = 3, DPH = inf (DPH adds src1.w)
//! and DP4 = 3 (0 * inf = 0).
//! 3. For (1, 1, 1, inf) . (1, 1, 1, 0) and (1, 1, 1, NaN) . (1, 1, 1, 1): DPH
//! ignores src0.w (3 and 4), DP4 gives 3 and 0x7FFFFFFF.
//! 4. 0x7FFFFFFF when products of +inf and -inf are summed, in any of the
//! three operations that include them.
//! 5. For (0, 0, 0, 0) . (inf, -inf, NaN, inf): DP3 = 0, DPH = inf, DP4 = 0.
//! 6. DP3 = 3 and DPH = DP4 = inf for (1, 1, 1, 1) . (1, 1, 1, inf).
//! 7. +0 from DP3 and DP4 for (-0, -0, -0, -0) . (1, 1, 1, 1).
struct TestVshExceptionalDot {
  static constexpr const char* Name() { return "VSH Exceptional DP3 DPH DP4"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates MIN with NaN, infinite, signed zero and denormal operands in both
//! operand orders.
//!
//! MIN, MAX, SLT and SGE compare the raw bits of their operands as
//! sign-magnitude integers. This is a total order: -NaN < -inf < negative
//! values < -0 < +0 < positive values < +inf < +NaN, with denormals ordered by
//! value rather than read as zero.
//!
//! Validates that:
//! 1. MIN(a, NaN) and MIN(NaN, a) return a for a positive NaN and the NaN
//! unaltered for a negative NaN, for each NaN encoding.
//! 2. MIN orders -inf < finite < +inf in either operand order.
//! 3. MIN(+0, -0) = MIN(-0, +0) = -0.
//! 4. MIN(-denormal, +0) = -denormal: the denormal is passed through.
struct TestVshExceptionalMin {
  static constexpr const char* Name() { return "VSH Exceptional MIN"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates MAX with NaN, infinite, signed zero and denormal operands in both
//! operand orders.
//!
//! Uses the sign-magnitude order described for TestVshExceptionalMin.
//!
//! Validates that:
//! 1. MAX(a, NaN) and MAX(NaN, a) return a positive NaN unaltered and return a
//! for a negative NaN, for each NaN encoding, including MAX(NaN, +inf).
//! 2. MAX orders -inf < finite < +inf in either operand order.
//! 3. MAX(+0, -0) = MAX(-0, +0) = +0.
//! 4. MAX(denormal, +0) = denormal: the denormal is passed through.
struct TestVshExceptionalMax {
  static constexpr const char* Name() { return "VSH Exceptional MAX"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates SLT with NaN, infinite, signed zero and denormal operands in both
//! operand orders.
//!
//! Uses the sign-magnitude order described for TestVshExceptionalMin.
//!
//! Validates that:
//! 1. SLT(x, +NaN) = 1 and SLT(+NaN, x) = 0 for non-NaN x, and
//! SLT(1.5, -NaN) = 0.
//! 2. SLT orders -inf < FLT_MAX < +inf and returns 0 for equal infinities.
//! 3. SLT(-0, +0) = 1 and SLT(+0, -0) = 0.
//! 4. A denormal is compared by value: SLT(-denormal, +0) = 1 and
//! SLT(denormal, 2^-126) = 1.
struct TestVshExceptionalSlt {
  static constexpr const char* Name() { return "VSH Exceptional SLT"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates SGE with NaN, infinite, signed zero and denormal operands in both
//! operand orders.
//!
//! Uses the sign-magnitude order described for TestVshExceptionalMin.
//!
//! Validates that:
//! 1. SGE(+NaN, x) = 1 and SGE(x, +NaN) = 0 for non-NaN x, SGE(NaN, NaN) = 1
//! for identical bits, and SGE(+inf, -NaN) = 1.
//! 2. SGE orders -inf < FLT_MAX < +inf and returns 1 for equal infinities.
//! 3. SGE(-0, +0) = 0 and SGE(+0, -0) = 1.
//! 4. A denormal is compared by value: SGE(+0, denormal) = 0 and
//! SGE(denormal, 2^-126) = 0.
struct TestVshExceptionalSge {
  static constexpr const char* Name() { return "VSH Exceptional SGE"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates DST with zero, infinite and NaN components.
//!
//! DST(a, b) = (1, a.y * b.y, a.z, b.w).
//!
//! Validates that DST produces:
//! 1. x = 1.0 regardless of NaN or infinite a.x and b.x.
//! 2. y = +0 for 0 * inf, inf * 0 and 0 * NaN whatever the signs, following
//! MUL.
//! 3. z = a.z and w = b.w unaltered, including NaN payloads, infinities and
//! -0.
struct TestVshExceptionalDst {
  static constexpr const char* Name() { return "VSH Exceptional DST"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_EXCEPTIONAL_MAC_H
