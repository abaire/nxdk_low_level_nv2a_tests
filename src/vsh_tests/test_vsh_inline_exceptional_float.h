#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_INLINE_EXCEPTIONAL_FLOAT_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_INLINE_EXCEPTIONAL_FLOAT_H

//! Validates how exceptional float values supplied through the inline vertex
//! methods (NV097_SET_VERTEX4F, NORMAL3F, DIFFUSE_COLOR4F, SPECULAR_COLOR4F
//! and FOG_COORD) arrive at the vertex shader inputs v0, v2, v3, v4 and v5.
//!
//! Ported from nxdk_vsh_tests (src/tests/exceptional_float_tests.cpp).
//!
//! Observed hardware behavior: every value, including NaN payloads and
//! denormals, reaches the shader input bit-exactly and is copied unchanged by
//! MOV. Denormals only behave as zero when consumed by arithmetic (see
//! "VSH LOG Denormal").
//!
//! Each value set is applied to every attribute (fog uses only the first
//! value; the normal only the first three).
//!
//! Validates that, for every inline attribute, the following are preserved
//! bit-exactly:
//! 1. +Inf and -Inf.
//! 2. Quiet NaNs of either sign (0x7FC00000, 0xFFC00000).
//! 3. +-FLT_MAX and +-2^-126 (smallest normal).
//! 4. Denormals (+-0x007FFFFF, +-0x00000001).
struct TestVshInlineExceptionalFloat {
  static constexpr const char* Name() {
    return "VSH Inline Attribute Exceptional Floats";
  }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_INLINE_EXCEPTIONAL_FLOAT_H
