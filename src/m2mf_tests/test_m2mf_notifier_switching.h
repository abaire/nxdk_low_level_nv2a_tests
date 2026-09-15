#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_NOTIFIER_SWITCHING_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_NOTIFIER_SWITCHING_H

//! Validates dynamic M2MF notifier DMA context switching via method 0x180
//! (DMA_NOTIFY).
//!
//! Validates that:
//! 1. Switching DMA_NOTIFY to a scratch notifier channel directs completion
//! records exclusively to scratch memory.
//! 2. The primary live notifier record is unmodified during scratch-targeted
//! transfers.
//! 3. Switching DMA_NOTIFY back to the live notifier channel restores live
//! notifications on subsequent transfers.
struct TestM2MFNotifierSwitching {
  static constexpr const char* Name() {
    return "M2MF Notifier Context Switching";
  }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_NOTIFIER_SWITCHING_H
