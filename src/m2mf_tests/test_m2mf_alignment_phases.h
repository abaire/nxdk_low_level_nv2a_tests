#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_ALIGNMENT_PHASES_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_ALIGNMENT_PHASES_H

//! Validates all 16 combinations of byte alignment phases (src % 4, dst % 4).
//!
//! Validates that:
//! 1. Memory transfers succeed across all combinations of misaligned source and
//! destination offsets.
//! 2. Pattern data is copied without byte corruption across phase boundaries.
//! 3. Pre-guard and post-guard boundary canaries remain intact regardless of
//! alignment offset.
struct TestM2MFAlignmentPhases {
  static constexpr const char* Name() { return "M2MF Alignment Phase Matrix"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_ALIGNMENT_PHASES_H
