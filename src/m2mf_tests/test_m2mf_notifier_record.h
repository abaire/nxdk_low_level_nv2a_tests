#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_NOTIFIER_RECORD_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_NOTIFIER_RECORD_H

//! Validates hardware completion notification record layout, status
//! transitions, and PTIMER timestamp reporting upon M2MF transfer completion.
//!
//! Validates that:
//! 1. Completion record status at offset 0x1C transitions from 0xFFFFFFFF
//! (armed) to 0 (success).
//! 2. Timestamps at offset 0x10 and 0x14 are non-zero and fall within the
//! sample window of active NV_PTIMER values.
//! 3. Hardware error/padding field at offset 0x18 remains 0 on successful
//! completion.
struct TestM2MFNotifierRecord {
  static constexpr const char* Name() {
    return "M2MF Notifier Completion Record";
  }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_NOTIFIER_RECORD_H
