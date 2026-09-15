#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_BATCHING_INDEPENDENT_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_BATCHING_INDEPENDENT_H

//! Validates batching multiple independent M2MF transfer commands in a single
//! pushbuffer frame.
//!
//! Validates that:
//! 1. Multiple back-to-back M2MF transfer commands execute without FIFO stalls.
//! 2. All batched chunks complete and match source patterns upon CPU sync.
//! 3. Destination memory for all independent chunks is properly updated.
struct TestM2MFBatchingIndependent {
  static constexpr const char* Name() {
    return "M2MF Batched Independent Transfers";
  }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_BATCHING_INDEPENDENT_H
