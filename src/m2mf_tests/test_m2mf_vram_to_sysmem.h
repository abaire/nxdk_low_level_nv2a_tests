#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_VRAM_TO_SYSMEM_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_VRAM_TO_SYSMEM_H

//! Validates DMA downloads/readbacks from Write-Combining (WC) video RAM to
//! Write-Back (WB) system memory.
//!
//! Validates that:
//! 1. Memory read back from VRAM into system RAM transfers accurately at DMA
//! speed.
//! 2. CPU cache invalidation (wbinvd) ensures CPU reads updated data from DRAM
//! rather than stale cache lines.
//! 3. Destination system memory guard canaries remain intact.
struct TestM2MFVramToSysmem {
  static constexpr const char* Name() {
    return "M2MF VRAM (WC) to Sysmem (WB)";
  }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_VRAM_TO_SYSMEM_H
