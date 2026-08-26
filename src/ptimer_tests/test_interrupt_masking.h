#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_INTERRUPT_MASKING_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_INTERRUPT_MASKING_H

//! Evaluates NV_PTIMER interrupt masking and pending bit behavior.
//!
//! Validates that:
//! 1. Disabling INTR_EN_0 in-flight prevents the CPU interrupt from firing.
//! 2. The pending bit in INTR_0 still latches when masked events occur.
//! 3. Re-enabling INTR_EN_0 while a pending bit is latched immediately asserts
//!    the CPU interrupt.
struct TestInterruptMasking {
  static constexpr const char* Name() { return "Interrupt Masking & Pending"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_INTERRUPT_MASKING_H
