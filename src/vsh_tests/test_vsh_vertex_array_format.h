#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_VERTEX_ARRAY_FORMAT_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_VERTEX_ARRAY_FORMAT_H

//! Validates how NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F data is expanded
//! into a vertex shader input, including exceptional values.
//!
//! Ported from nxdk_vsh_tests (src/tests/vertex_data_array_format_tests.cpp,
//! "Float"). Expected values are the bit patterns observed on hardware.
//!
//! Validates that the following are passed through bit-exactly:
//! 1. (0, 1, -1, 0.99999999f).
//! 2. +-Inf and quiet NaNs of either sign (0x7FC00000, 0xFFC00000).
//! 3. +-FLT_MAX and +-2^-126 (smallest normal).
//! 4. Denormals (+-0x007FFFFF, +-0x00000001).
struct TestVshVertexArrayFormatF {
  static constexpr const char* Name() { return "VSH Vertex Array Format F"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates how NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S1 (normalized signed
//! short) data is expanded into a vertex shader input.
//!
//! Ported from nxdk_vsh_tests (src/tests/vertex_data_array_format_tests.cpp,
//! "S1"). Expected values are the bit patterns observed on hardware, which
//! follow v -> (2v + 1) / 65535. -511/65535 and -65533/65535 are one ULP
//! closer to zero than the IEEE round-to-nearest quotient.
//!
//! Validates that:
//! 1. (32767, -32768, 256, -256) expands to
//! (0x3F800000, 0xBF800000, 0x3C004080, 0xBBFF80FF), i.e.
//! (1, -1, 513/65535, -511/65535).
//! 2. (1, 0, -1, -32767) expands to
//! (0x384000C0, 0x37800080, 0xB7800080, 0xBF7FFDFF), i.e.
//! (3/65535, 1/65535, -1/65535, -65533/65535).
struct TestVshVertexArrayFormatS1 {
  static constexpr const char* Name() { return "VSH Vertex Array Format S1"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

//! Validates how NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S32K (unnormalized
//! signed short) data is expanded into a vertex shader input.
//!
//! Ported from nxdk_vsh_tests (src/tests/vertex_data_array_format_tests.cpp,
//! "S32K"). Expected values are the bit patterns observed on hardware.
//!
//! Validates that:
//! 1. (32767, -32768, 256, -256) and (1, 0, -1, -32767) expand to exactly the
//! same values as floats.
struct TestVshVertexArrayFormatS32K {
  static constexpr const char* Name() { return "VSH Vertex Array Format S32K"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_VERTEX_ARRAY_FORMAT_H
