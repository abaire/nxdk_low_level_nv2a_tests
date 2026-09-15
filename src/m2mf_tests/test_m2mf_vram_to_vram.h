#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_VRAM_TO_VRAM_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_VRAM_TO_VRAM_H

//! Validates VRAM-to-VRAM DMA transfers using gpum_copy_wc (Write-Combining
//! store fence).
//!
//! Validates that:
//! 1. Memory transfers between two distinct Write-Combining video memory
//! buffers copy accurately.
//! 2. Fast store fence synchronization (sfence) completes without requiring a
//! full CPU cache flush (wbinvd).
//! 3. Destination guard canaries remain uncorrupted.
struct TestM2MFVramToVram {
  static constexpr const char* Name() { return "M2MF VRAM (WC) to VRAM (WC)"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_VRAM_TO_VRAM_H
