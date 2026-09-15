#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_ALIGNMENT_ODD_SIZES_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_ALIGNMENT_ODD_SIZES_H

//! Validates non-power-of-two, odd, and prime transfer sizes across misaligned
//! byte offsets.
//!
//! Validates that:
//! 1. Arbitrary non-power-of-two sizes (1..1027 bytes) copy completely and
//! accurately.
//! 2. Head and tail unaligned byte fragments are handled cleanly by hardware
//! DMA.
//! 3. Surrounding destination memory canaries remain intact without buffer
//! overruns or underruns.
struct TestM2MFAlignmentOddSizes {
  static constexpr const char* Name() {
    return "M2MF Alignment Odd & Prime Sizes";
  }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_ALIGNMENT_ODD_SIZES_H
