#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_BATCHING_CAUSAL_CHAIN_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_BATCHING_CAUSAL_CHAIN_H

//! Validates in-order FIFO execution of chained causal transfers (A -> B
//! followed by B -> C) within a single pushbuffer frame without CPU
//! intervention.
//!
//! Validates that:
//! 1. The NV2A hardware executes consecutive M2MF transfers in strict FIFO
//! order.
//! 2. Buffer B is fully populated from Buffer A before the B -> C transfer
//! reads from Buffer B.
//! 3. No read-after-write (RAW) hazard occurs between dependent transfers in
//! the same pushbuffer.
struct TestM2MFBatchingCausalChain {
  static constexpr const char* Name() {
    return "M2MF Causal Chained Transfers";
  }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_BATCHING_CAUSAL_CHAIN_H
