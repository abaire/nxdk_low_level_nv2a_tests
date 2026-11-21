#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_EXCEPTIONAL_ILU_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_EXCEPTIONAL_ILU_H

// Tests of ILU operations and ARL on signed zeros, infinities, NaNs and values
// at the edges of the float32 range.
//
// Except for LIT, each ILU shader reads its input directly from a component of
// c[96] and writes the result directly to a constant, so no MOV is involved.
// Expectations are hardware results, logged with alternative models as
// described in test_vsh_exceptional_mac.h. Denormal inputs are read as zero
// (TestVshLogDenormal).
//
// The nv2avsh assembler accepts only the EXPP and LOGP spellings of the
// EXP and LOG opcodes, so EXP/EXPP and LOG/LOGP are each covered by one test.

//! Validates RCP at signed zeros, infinities, NaNs and the edges of the float32
//! range.
//!
//! Validates that RCP produces:
//! 1. +inf for +0 and -inf for -0, as IEEE 754 does ("pos_inf" is the D3D
//! definition, +inf for both).
//! 2. +0 for +inf and -0 for -inf.
//! 3. 0x7FFFFFFF for every NaN encoding.
//! 4. 1, 0.5 and -0.5 for 1, 2 and -2: reciprocals of powers of two are
//! exact.
//! 5. +-0 for +-FLT_MAX, flushing the denormal result to a zero of the same
//! sign.
//! 6. 2^126 for 2^-126, +inf for the denormal 0x00400000 and -inf for
//! 0x80400000: a denormal is read as a zero of the same sign.
struct TestVshExceptionalRcp {
  static constexpr const char* Name() { return "VSH Exceptional RCP"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates RCC at signed zeros, infinities, NaNs and its clamp limits.
//!
//! RCC clamps the reciprocal to [2^-64, 2^64] in magnitude.
//!
//! Validates that RCC produces:
//! 1. 2^64 for +0, 2^-70, 2^-126 and the denormal 0x00400000.
//! 2. -2^64 for -0 and -2^-70, consistent with RCP(-0) = -inf ("pos" gives
//! +2^64).
//! 3. 2^-64 for +inf, FLT_MAX and 2^70, and -2^-64 for -inf.
//! 4. 0x7FFFFFFF for every NaN encoding: NaN is not clamped ("clamp_max" gives
//! 2^64).
//! 5. 1, 0.5 and -0.5 for 1, 2 and -2.
struct TestVshExceptionalRcc {
  static constexpr const char* Name() { return "VSH Exceptional RCC"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates RSQ at signed zeros, infinities, NaNs, negative inputs and the
//! edges of the float32 range.
//!
//! Validates that RSQ produces:
//! 1. +inf for +0 and -0.
//! 2. +0 for +inf and -inf.
//! 3. 0x7FFFFFFF for every NaN encoding.
//! 4. 1, 0.5 and 2 for 1, 4 and 0.25, and 0.5 for -4: RSQ(x) = 1 / sqrt(|x|)
//! ("ieee" gives -inf for -0 and NaN for negative inputs).
//! 5. 2^63 for 2^-126 and 2^-63 for 2^126.
//! 6. +inf for the denormals 0x00400000 and 0x80400000.
struct TestVshExceptionalRsq {
  static constexpr const char* Name() { return "VSH Exceptional RSQ"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates EXPP (the EXP opcode) at signed zeros, infinities and NaNs.
//!
//! EXPP(s) = (2^floor(s), s - floor(s), ~2^s, 1).
//!
//! Validates that EXPP produces:
//! 1. (1, +0, 1, 1) for +0, -0 and the denormal 0x00400000.
//! 2. (0.25, 0.5, 0x3EB50600, 1) for -1.5 and (4, 0, 4, 1) for 2, including
//! the hardware approximation of 2^-1.5 in z.
//! 3. (+inf, +0, +inf, 1) for +inf: the fraction of an infinity is +0
//! ("ieee" gives NaN).
//! 4. (+0, +0, +0, 1) for -inf.
//! 5. (+inf, s, 0x7FFFFFFF, 1) for every NaN s: x is +inf whatever the NaN's
//! sign, y passes the NaN input through unaltered and z is the canonical NaN.
struct TestVshExceptionalExpp {
  static constexpr const char* Name() { return "VSH Exceptional EXPP"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates LOGP (the LOG opcode) at signed zeros, infinities and NaNs.
//!
//! LOGP(s) = (floor(log2|s|), |s| / 2^floor(log2|s|), ~log2|s|, 1).
//!
//! Validates that LOGP produces:
//! 1. (-inf, 1, -inf, 1) for +0, -0 and the denormal 0x00400000.
//! 2. (0, 1, 0, 1), (3, 1, 3, 1), (3, 1, 3, 1), (0, 1.5, 0x3F15C780, 1) and
//! (127, 0x3FFFFFFF, 0x42FFFFF3, 1) for 1, 8, -8, 1.5 and FLT_MAX, including
//! the hardware approximation of log2 in z.
//! 3. (+inf, 1, +inf, 1) for +inf and -inf ("bitfield" gives the biased
//! exponent field 128 in x).
//! 4. (s, s, s, 1) for the NaNs s = 0x7FFFFFFF and 0x7FC00000: the NaN input is
//! passed through unaltered in x, y and z ("canon_nan" gives 0x7FFFFFFF).
struct TestVshExceptionalLogp {
  static constexpr const char* Name() { return "VSH Exceptional LOGP"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates LIT with zero, infinite and NaN N.L, N.H and specular power
//! inputs.
//!
//! LIT(s) = (1, max(s.x, 0), s.x > 0 ? max(s.y, 0)^s.w : 0, 1) with s.w clamped
//! to about +-128. Each input is s = (N.L, N.H, 0, power). The max and the
//! s.x > 0 gate use the sign-magnitude order of MAX and SLT (see
//! TestVshExceptionalMin), so a positive NaN N.L is greater than 0.
//!
//! Validates that LIT produces:
//! 1. z = 1 for N.L = 1, N.H = 0 and power 0: 0^0 = 1 ("ieee" gives NaN,
//! "d3d" 0).
//! 2. z = 1 for N.H = 0.5 and power 0 (control).
//! 3. z = 0 for N.H = 0 and power 2 or +inf.
//! 4. z = +inf for N.H = 0 and power -1.
//! 5. z = 0 for N.H = 0.5 and power +inf: the clamped power gives a result
//! below 2^-126, flushed to zero.
//! 6. z = 0x7F7FF88A (about 2^127.99) for N.H = 2 and power +inf: the power is
//! clamped ("no_clamp" gives +inf).
//! 7. z = 0 and y = +0 when N.L is 0 or -0.
//! 8. y = N.L unaltered and z = N.H^power = 0.5 when N.L is the positive NaN
//! 0x7FC00000 ("clamped" gives y = +0, "gated" z = 0).
//! 9. z = 0 for N.H = -0.5.
//! 10. z = 1 for N.H = NaN and power 0.
//! 11. y = +inf and z = 0.5 for N.L = +inf, N.H = 0.5 and power 1.
//! 12. x = w = 1 for every input.
struct TestVshExceptionalLit {
  static constexpr const char* Name() { return "VSH Exceptional LIT"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates how ARL converts infinite, NaN and huge values to an address.
//!
//! Each probe loads A0 with ARL from the raw bits in iTex0.x and draws c[A0]
//! as its color (see TestVshA0BeforeStart). The constants are loaded with
//! c[i] = (i / 255, 1, 0.5, 1), so the color identifies the constant read.
//! NV_PGRAPH_INTR is logged after the draw.
//!
//! Validates that:
//! 1. ARL of 1, 1.5, 5, +0 and -0 reads c[1], c[1], c[5], c[0] and c[0]
//! (controls).
//! 2. ARL of the denormals 0x00400000 and 0x80400000 reads c[0]: the negative
//! denormal is read as -0, so floor gives 0 rather than -1.
//! 3. ARL of +inf, -inf, the NaNs 0x7FFFFFFF, 0x7FC00000 and 0xFFC00000, and
//! +-FLT_MAX produces an out of range address, which reads as zero.
struct TestVshExceptionalArl {
  static constexpr const char* Name() { return "VSH Exceptional ARL"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_EXCEPTIONAL_ILU_H
