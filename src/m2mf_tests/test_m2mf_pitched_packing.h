#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_PITCHED_PACKING_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_PITCHED_PACKING_H

//! Validates row packing and unpacking (converting between dense and pitched
//! memory layouts).
//!
//! Validates that:
//! 1. Unpacking rows (pitch_in == line_len, pitch_out > line_len) copies dense
//! rows into a strided destination without corrupting padding.
//! 2. Packing rows (pitch_in > line_len, pitch_out == line_len) extracts
//! strided source rows into a contiguous destination buffer.
struct TestM2MFPitchedPacking {
  static constexpr const char* Name() {
    return "M2MF Pitched Row Packing/Unpacking";
  }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_PITCHED_PACKING_H
