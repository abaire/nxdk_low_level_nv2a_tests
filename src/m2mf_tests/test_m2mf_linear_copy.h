#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_LINEAR_COPY_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_LINEAR_COPY_H

//! Validates GPU-accelerated memory-to-memory copy using NV2A Class 0x39 (M2MF)
//! via the pbkitplusplus gpum interface.
//!
//! Validates that:
//! 1. Memory copy of a 4 KB contiguous buffer completes successfully without
//! error.
//! 2. Copied destination data matches source data byte-for-byte.
//! 3. Pre-guard and post-guard canary bytes surrounding the destination buffer
//! remain unmodified.
//! 4. Source memory buffer remains unmodified throughout the transfer.
struct TestM2MFLinearCopy {
  static constexpr const char* Name() { return "M2MF Linear Copy"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_LINEAR_COPY_H
