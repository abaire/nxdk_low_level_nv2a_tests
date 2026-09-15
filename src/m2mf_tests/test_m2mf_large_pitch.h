#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_LARGE_PITCH_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_LARGE_PITCH_H

//! Validates that the M2MF engine correctly handles large pitch strides up to
//! 16384 bytes without register overflow or row calculation errors.
//!
//! Validates that:
//! 1. Multi-line transfers with a large pitch stride (16384 B) step accurately
//! row-by-row.
//! 2. Line payload data matches expected source patterns across all lines.
//! 3. Large inter-line gaps (16384 B - 64 B) in destination memory remain
//! untouched.
struct TestM2MFLargePitch {
  static constexpr const char* Name() {
    return "M2MF Large Pitch Stride (16 KB)";
  }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_LARGE_PITCH_H
