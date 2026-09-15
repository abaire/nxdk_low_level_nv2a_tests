#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_PITCHED_ASYMMETRIC_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_PITCHED_ASYMMETRIC_H

//! Validates 2D strided copies with asymmetric pitch (pitch_in != pitch_out).
//!
//! Validates that:
//! 1. Transfers with pitch_in > pitch_out step source and destination
//! accurately.
//! 2. Transfers with pitch_in < pitch_out step source and destination
//! accurately.
//! 3. Destination inter-line padding bytes remain unmodified in both
//! configurations.
struct TestM2MFPitchedAsymmetric {
  static constexpr const char* Name() { return "M2MF Pitched 2D Asymmetric"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_PITCHED_ASYMMETRIC_H
