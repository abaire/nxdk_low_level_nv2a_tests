#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_MIN_TRANSFER_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_MIN_TRANSFER_H

//! Validates the minimum allowable M2MF transfer size of a single byte.
//!
//! Validates that:
//! 1. A 1x1 byte transfer copies the single byte payload accurately.
//! 2. Pre-guard and post-guard boundary canaries in destination memory remain
//! untouched.
//! 3. The transfer signals the hardware completion notifier without hanging the
//! GPU.
struct TestM2MFMinTransfer {
  static constexpr const char* Name() {
    return "M2MF Minimum Transfer (1 Byte)";
  }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_MIN_TRANSFER_H
