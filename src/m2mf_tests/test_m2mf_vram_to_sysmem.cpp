#include "test_m2mf_vram_to_sysmem.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

bool TestM2MFVramToSysmem::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF VRAM (WC) to Sysmem (WB)\n");

  if (!M2MFInit()) {
    LogMsg("  [FAIL] Failed to initialize M2MF engine.\n");
    return false;
  }

  constexpr size_t kCanarySize = 256;
  constexpr size_t kDataSize = 16384;
  constexpr size_t kAllocSize = kDataSize + kCanarySize * 2;

  auto sysmem_buf = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kAllocSize, 0, MAXRAM, 0, PAGE_READWRITE));
  auto vram_buf = static_cast<uint8_t*>(MmAllocateContiguousMemoryEx(
      kAllocSize, 0, MAXRAM, 0, PAGE_READWRITE | PAGE_WRITECOMBINE));

  if (!sysmem_buf || !vram_buf) {
    LogMsg("  [FAIL] Failed to allocate test memory buffers.\n");
    if (sysmem_buf) MmFreeContiguousMemory(sysmem_buf);
    if (vram_buf) MmFreeContiguousMemory(vram_buf);
    return false;
  }

  uint8_t* src = vram_buf + kCanarySize;
  uint8_t* dst = sysmem_buf + kCanarySize;
  constexpr uint8_t kSeed = 0x5F;

  std::memset(sysmem_buf, 0x55, kCanarySize);
  std::memset(dst, 0x00, kDataSize);
  std::memset(dst + kDataSize, 0xAA, kCanarySize);

  M2MFFillPattern(src, kDataSize, kSeed);

  bool pass = true;
  if (!gpum_copy(dst, src, kDataSize)) {
    LogMsg("  [FAIL] VRAM -> Sysmem download timed out or failed.\n");
    pass = false;
  } else if (!M2MFVerifyPattern(dst, kDataSize, kSeed)) {
    LogMsg("  [FAIL] VRAM -> Sysmem download data mismatch.\n");
    pass = false;
  } else if (!M2MFVerifyCanaries(sysmem_buf, kCanarySize, 0x55) ||
             !M2MFVerifyCanaries(dst + kDataSize, kCanarySize, 0xAA)) {
    LogMsg("  [FAIL] VRAM -> Sysmem download corrupted canaries.\n");
    pass = false;
  } else {
    LogMsg("  [PASS] VRAM (WC) -> Sysmem (WB) download (16 KB) verified.\n");
  }

  MmFreeContiguousMemory(sysmem_buf);
  MmFreeContiguousMemory(vram_buf);
  return pass;
}
