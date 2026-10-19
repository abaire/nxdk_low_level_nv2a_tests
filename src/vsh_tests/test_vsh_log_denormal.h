#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_LOG_DENORMAL_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_LOG_DENORMAL_H

//! Characterizes how LOGP treats denormal inputs: whether a denormal reaches
//! the shader intact, and what exponent/mantissa/log2 LOGP returns for it.
//!
//! Inputs are built from raw bit patterns and delivered unmodified through
//! both a vertex attribute (iTex0.x) and a constant register (c[190].x). For
//! each path a LOGP shader encodes its result in the diffuse color:
//! red = exponent + 160, green = (mantissa - 1) * 255, blue = log2 + 160. A
//! second, pass-through shader multiplies the input by 2^126 and 2^149 to show
//! whether the denormal survives as a non-zero value.
//!
//! Observed hardware behavior: denormals are treated as zero when consumed as
//! an arithmetic operand, identically for the attribute and constant paths.
//! Every
//! denormal probe (0x007FFFFF, 0x00400000, 0x00000001 and 0x80000001) yields
//! 0 from the 2^126 / 2^149 multiply and LOGP returns the same result as for
//! +0 (x = -inf, y = 1, z = -inf), including for the negative denormal. The
//! smallest normal (0x00800000) is unaffected: LOGP gives exponent -126,
//! mantissa 1.0 and log2 -126. Because the multiply also reads zero, LOGP is
//! not itself flushing; the operand is read as zero by both the MAC and ILU.
//! The register itself still holds the denormal: MOV copies it bit-exactly
//! (see "VSH Inline Attribute Exceptional Floats" and
//! "VSH Vertex Array Format F"). An emulator should therefore treat denormal
//! operands of arithmetic operations as signed zero without altering the
//! stored value.
//!
//! Validates that:
//! 1. Zero produces LOGP (x, y, z) = (-inf, 1, -inf), observed as black.
//! 2. The smallest normal (0x00800000) produces exponent -126 and log2 -126.
//! 3. Normal and zero inputs are passed through the multiply shaders as
//! expected (0x00800000 * 2^126 == 1, 0 * 2^126 == 0).
//! 4. Each denormal (0x007FFFFF, 0x00400000, 0x00000001, 0x80000001) is
//! reported as one of the observable outcomes: read as zero by every operation,
//! read as zero by LOG only, preserved with a field-based exponent (-127), or
//! preserved and fully normalized (floor(log2(|v|))).
//! 5. Every denormal probe is consistent with the outcome identified by
//! 0x00000001, and the attribute and constant paths agree.
struct TestVshLogDenormal {
  static constexpr const char* Name() { return "VSH LOG Denormal"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_LOG_DENORMAL_H
