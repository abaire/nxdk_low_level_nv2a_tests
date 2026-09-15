#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_ZERO_SIZE_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_ZERO_SIZE_H

//! Validates that zero line count or zero line length requests complete
//! gracefully without hanging the GPU, stalling the FIFO, or modifying
//! destination memory.
//!
//! Validates that:
//! 1. A transfer request with line count = 0 completes immediately and leaves
//! destination memory untouched.
//! 2. A transfer request with line length = 0 completes immediately and leaves
//! destination memory untouched.
//! 3. Neither operation hangs the pushbuffer or triggers GPU MMIO error
//! interrupts.
struct TestM2MFZeroSize {
  static constexpr const char* Name() { return "M2MF Zero Count / Length"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_ZERO_SIZE_H
