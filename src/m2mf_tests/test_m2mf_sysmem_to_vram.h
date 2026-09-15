#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_SYSMEM_TO_VRAM_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_SYSMEM_TO_VRAM_H

//! Validates DMA uploads from Write-Back (WB) cached system memory to
//! Write-Combining (WC) video RAM.
//!
//! Validates that:
//! 1. Memory uploaded from cached system RAM to uncached VRAM transfers
//! accurately.
//! 2. CPU cache flushing (wbinvd) ensures GPU fetches dirty system RAM lines.
//! 3. Destination VRAM guard canaries remain uncorrupted.
struct TestM2MFSysmemToVram {
  static constexpr const char* Name() {
    return "M2MF Sysmem (WB) to VRAM (WC)";
  }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_SYSMEM_TO_VRAM_H
