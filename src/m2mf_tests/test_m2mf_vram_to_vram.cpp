#include "test_m2mf_vram_to_vram.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

bool TestM2MFVramToVram::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF VRAM (WC) to VRAM (WC)\n");

  if (!M2MFInit()) {
    LogMsg("  [FAIL] Failed to initialize M2MF engine.\n");
    return false;
  }

  constexpr size_t kCanarySize = 256;
  constexpr size_t kDataSize = 16384;
  constexpr size_t kAllocSize = kDataSize + kCanarySize * 2;

  auto vram1_buf = static_cast<uint8_t*>(MmAllocateContiguousMemoryEx(
      kAllocSize, 0, MAXRAM, 0, PAGE_READWRITE | PAGE_WRITECOMBINE));
  auto vram2_buf = static_cast<uint8_t*>(MmAllocateContiguousMemoryEx(
      kAllocSize, 0, MAXRAM, 0, PAGE_READWRITE | PAGE_WRITECOMBINE));

  if (!vram1_buf || !vram2_buf) {
    LogMsg("  [FAIL] Failed to allocate test memory buffers.\n");
    if (vram1_buf) MmFreeContiguousMemory(vram1_buf);
    if (vram2_buf) MmFreeContiguousMemory(vram2_buf);
    return false;
  }

  uint8_t* src = vram1_buf + kCanarySize;
  uint8_t* dst = vram2_buf + kCanarySize;
  constexpr uint8_t kSeed = 0x91;

  std::memset(vram2_buf, 0x55, kCanarySize);
  std::memset(dst, 0x00, kDataSize);
  std::memset(dst + kDataSize, 0xAA, kCanarySize);

  M2MFFillPattern(src, kDataSize, kSeed);

  bool pass = true;
  if (!gpum_copy_wc(dst, src, kDataSize)) {
    LogMsg("  [FAIL] VRAM -> VRAM transfer timed out or failed.\n");
    pass = false;
  } else if (!M2MFVerifyPattern(dst, kDataSize, kSeed)) {
    LogMsg("  [FAIL] VRAM -> VRAM transfer data mismatch.\n");
    pass = false;
  } else if (!M2MFVerifyCanaries(vram2_buf, kCanarySize, 0x55) ||
             !M2MFVerifyCanaries(dst + kDataSize, kCanarySize, 0xAA)) {
    LogMsg("  [FAIL] VRAM -> VRAM transfer corrupted canaries.\n");
    pass = false;
  } else {
    LogMsg("  [PASS] VRAM (WC) -> VRAM (WC) transfer (16 KB) verified.\n");
  }

  MmFreeContiguousMemory(vram1_buf);
  MmFreeContiguousMemory(vram2_buf);
  return pass;
}
