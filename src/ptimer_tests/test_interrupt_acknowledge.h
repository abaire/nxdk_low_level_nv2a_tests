#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_INTERRUPT_ACKNOWLEDGE_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_INTERRUPT_ACKNOWLEDGE_H

//! Evaluates NV_PTIMER interrupt acknowledgment / Write-1-to-Clear semantics.
//!
//! Validates that:
//! 1. Writing 0 to a pending bit in INTR_0 has no effect (the pending bit
//! remains set).
//! 2. Writing 1 to a pending bit in INTR_0 acknowledges and clears the bit to
//! 0.
struct TestInterruptAcknowledge {
  static constexpr const char* Name() { return "Interrupt Acknowledge (W1C)"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_INTERRUPT_ACKNOWLEDGE_H
