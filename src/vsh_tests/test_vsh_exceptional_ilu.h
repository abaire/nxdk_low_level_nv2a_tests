#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_EXCEPTIONAL_ILU_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_EXCEPTIONAL_ILU_H

// Tests of ILU operations and ARL on signed zeros, infinities, NaNs and values
// at the edges of the float32 range.
//
// Except for LIT, each ILU shader reads its input directly from a component of
// c[96] and writes the result directly to a constant, so no MOV is involved.
// Expectations are hardware results. Denormal inputs are read as zero
// (TestVshLogDenormal).
//
// The nv2avsh assembler accepts only the EXPP and LOGP spellings of the
// EXP and LOG opcodes, so EXP/EXPP and LOG/LOGP are each covered by one test.

//! Validates RCP at signed zeros, infinities, NaNs and the edges of the float32
//! range.
//!
//! Validates that RCP produces:
//! 1. +inf for +0 and -inf for -0.
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
//! 2. -2^64 for -0 and -2^-70, consistent with RCP(-0) = -inf.
//! 3. 2^-64 for +inf, FLT_MAX and 2^70, and -2^-64 for -inf.
//! 4. 0x7FFFFFFF for every NaN encoding: NaN is not clamped.
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
//! 4. 1, 0.5 and 2 for 1, 4 and 0.25, and 0.5 for -4: RSQ(x) = 1 / sqrt(|x|).
//! 5. 2^63 for 2^-126 and 2^-63 for 2^126.
//! 6. +inf for the denormals 0x00400000 and 0x80400000.
struct TestVshExceptionalRsq {
  static constexpr const char* Name() { return "VSH Exceptional RSQ"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates EXPP (the EXP opcode) at signed zeros, infinities, NaNs and
//! small negative inputs.
//!
//! EXPP(s) = (2^floor(s), s - floor(s), ~2^s, 1).
//!
//! Validates that EXPP produces:
//! 1. (1, +0, 1, 1) for +0, -0 and the denormal 0x00400000.
//! 2. (0.25, 0.5, 0x3EB50600, 1) for -1.5 and (4, 0, 4, 1) for 2, including
//! the hardware approximation of 2^-1.5 in z.
//! 3. (+inf, +0, +inf, 1) for +inf: the fraction of an infinity is +0.
//! 4. (+0, +0, +0, 1) for -inf.
//! 5. (+inf, s, 0x7FFFFFFF, 1) for every NaN s: x is +inf whatever the NaN's
//! sign, y passes the NaN input through unaltered and z is the canonical NaN.
//! 6. (1, 1, 1, 1) for negative inputs smaller in magnitude than 2^-23:
//! -1e-10, -2^-25, -2^-24, 0xB3800001, -1.5 * 2^-24, -15 * 2^-27 and
//! 0xB3FFFFFF, the largest such input.
//! 7. x = 2^floor(s) and y = s - floor(s) for -2^-23, -2^-22, -2^-20,
//! -1 - 2^-23 and -2 - 2^-22, with z the hardware approximation of 2^s.
//! 8. A negative input is rounded toward -inf to a multiple of 2^-23 before
//! the fraction is taken: y = 0x3F7FFFFC for 0xB4000001 (one ulp beyond
//! -2^-23), -1.5 * 2^-23, -17 * 2^-27 and -31 * 2^-27, 0x3EFFFFFC for -0.5 -
//! 2^-24 and 0x3F3FFFFE for -0.25 - 2^-25.
//! 9. y = s unaltered for the positive 2^-24: positive inputs are not rounded.
//! 10. z approximates 2^fraction with a piecewise table that jumps at every
//! multiple of 1/64 sampled: 1 + 32/64 - 2^-23 gives 0x4035021E but 1 + 32/64
//! gives 0x40350600. Among the 128 points sampled at and just below each
//! multiple of 1/64, the largest errors are -1.14e-4 just below a fraction of
//! 1 (as for -2^-23), -1.06e-4 at 1 + 4/64 - 2^-23 and +9.7e-5 at 1 + 61/64.
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
//! 3. (+inf, 1, +inf, 1) for +inf and -inf.
//! 4. (s, s, s, 1) for the NaNs s = 0x7FFFFFFF and 0x7FC00000: the NaN input is
//! passed through unaltered in x, y and z.
struct TestVshExceptionalLogp {
  static constexpr const char* Name() { return "VSH Exceptional LOGP"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates LIT with zero, denormal, infinite and NaN N.L, N.H and specular
//! power inputs.
//!
//! LIT(s) = (1, max(s.x, 0), s.x > 0 ? max(s.y, 0)^s.w : 0, 1) with s.w clamped
//! to +-128 and the exponentiation approximated. Each input is s = (N.L, N.H,
//! 0, power). The max and the s.x > 0 gate use the sign-magnitude order of MAX
//! and SLT (see TestVshExceptionalMin), so a positive NaN N.L is greater than
//! 0.
//!
//! Validates that LIT produces:
//! 1. z = 1 for N.L = 1, N.H = 0 and power 0: 0^0 = 1.
//! 2. z = 1 for N.H = 0.5 and power 0 (control).
//! 3. z = 0 for N.H = 0 and power 2 or +inf.
//! 4. z = +inf for N.H = 0 and power -1.
//! 5. z = 0 for N.H = 0.5 and power +inf: the clamped power gives a result
//! below 2^-126, flushed to zero.
//! 6. z = 0x7F7FF88A (about 2^127.9998) for N.H = 2 and power +inf: the power
//! is clamped to 128 (see 22).
//! 7. z = 0 and y = +0 when N.L is 0 or -0.
//! 8. y = N.L unaltered and z = N.H^power = 0.5 when N.L is the positive NaN
//! 0x7FC00000.
//! 9. z = 0 for N.H = -0.5.
//! 10. z = 1 for N.H = NaN and power 0.
//! 11. y = +inf and z = 0.5 for N.L = +inf, N.H = 0.5 and power 1.
//! 12. x = w = 1 for every input.
//! 13. Denormal N.H and power inputs are treated as +0: for a denormal N.H,
//! z = 0 with power 0.5, 1 with power 0 and +inf with power -1, and z = 1 for
//! N.H = 0 with a denormal power of either sign.
//! 14. y = N.L unaltered but z = 0 for a denormal N.L, y = +0 for a negative
//! denormal N.L and z = 0.5 for N.L = 2^-126.
//! 15. A negative N.H (-0, -denormal or -0.5) is replaced by +0: z = 1 with
//! power 0 and +inf with power -1.
//! 16. y = +0 and z = 0 for the negative NaN N.L 0xFFC00000.
//! 17. z = 0x7FFFFFFF for the negative NaN N.H 0xFFC00000 with power -1 and
//! for the NaN power 0x7FC00000.
//! 18. y = N.L unaltered for N.L = 0.5, 2, FLT_MAX and 0x7FFFFFFF, and y = +0
//! and z = 0 for N.L = -1, -inf and -FLT_MAX. The negative NaN N.L 0xFFC00000
//! gives z = 0 even with N.H = -0 and power -1.
//! 19. s.z is ignored: z = 0.5 for s.z = NaN or +inf.
//! 20. The exponentiation is approximate. 0.5^2, 0.5^-1, 2^3, 1^2 and 1^-1 are
//! exact, but z = 0x3F350600 for 0.5^0.5, 0x3FB50600 for 2^0.5, 0x41100520 for
//! 3^2, 0x40100520 for 1.5^2 and 0x3D66D73A for 0.75^10.
//! 21. At the edges of the float32 range, z = 2^127 for 2^127, 0x7F350600 for
//! 2^127.5, 0x7F7FF88A for 2^128 and FLT_MAX^1, +inf for 4^64, 4^100 and
//! FLT_MAX^2, 2^-126 for 2^-126, and 0 for 2^-126.5, 2^-128 and 0.25^100.
//! 22. The power is clamped to +-128. For N.H = sqrt(2), powers 128, 129, 200
//! and +inf all give 0x5F804BBA and powers -128, -200 and -inf all give
//! 0x1F7F6518, while 127, 127.9961, 127.9998 and -127.9961 give 0x5F3570F2,
//! 0x5F801EDA, 0x5F8048EC and 0x1F7FB702.
//! 23. For power -inf, z = +inf for N.H = 0 and 0.5 and 0 for N.H = 2. z = 1
//! for N.H = 1 with power +inf or -inf.
//! 24. For N.H = +inf, z = +inf with power 1 or 0.5, 1 with power 0 and 0 with
//! power -1. N.H = -inf, -FLT_MAX and -2 are replaced by +0: z = 0 with power 1
//! or 2, 1 with power 0 and +inf with power -1.
//! 25. A NaN N.H propagates regardless of sign: z = 0x7FFFFFFF for N.H =
//! 0x7FC00000 with power 1 or -1, 0x7FA5A5A5 with power 1 and 0xFFC00000 with
//! power 1. A zero power still gives 1: z = 1 for N.H = 0x7FC00000 with a
//! denormal power, 0x7FFFFFFF with power 0 and 0xFFC00000 with power 0 or
//! -denormal.
//! 26. z = 0x7FFFFFFF for every NaN power (0x7FC00000, 0xFFC00000, 0x7FA5A5A5),
//! including with N.H = 0, 1, -0.5, +inf and NaN.
//! 27. y = +0 (or the unaltered denormal N.L) and z = 0 whenever N.L is not
//! greater than 0 after flushing: for N.L = 0, -0, -1, a denormal and
//! 0xFFC00000, even with a NaN power, a NaN N.H, 0^-1 or +inf^1.
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
