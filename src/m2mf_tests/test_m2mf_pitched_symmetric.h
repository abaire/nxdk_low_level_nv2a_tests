#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_PITCHED_SYMMETRIC_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_PITCHED_SYMMETRIC_H

//! Validates 2D rectangular strided copies with identical source and
//! destination pitch (pitch_in == pitch_out > line_len).
//!
//! Validates that:
//! 1. Each row of line_len bytes is copied accurately to the destination row.
//! 2. Inter-line padding bytes in destination (pitch_out - line_len) remain
//! untouched.
//! 3. Destination pre-guard and post-guard boundary canaries remain intact.
struct TestM2MFPitchedSymmetric {
  static constexpr const char* Name() { return "M2MF Pitched 2D Symmetric"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_PITCHED_SYMMETRIC_H
