#include "test_m2mf_sysmem_to_vram.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

bool TestM2MFSysmemToVram::Test() {
  if (!M2MFInit()) {
    TEST_FAIL("Failed to initialize M2MF engine.");
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
    TEST_FAIL("Failed to allocate test memory buffers.");
    if (sysmem_buf) MmFreeContiguousMemory(sysmem_buf);
    if (vram_buf) MmFreeContiguousMemory(vram_buf);
    return false;
  }

  uint8_t* src = sysmem_buf + kCanarySize;
  uint8_t* dst = vram_buf + kCanarySize;
  constexpr uint8_t kSeed = 0x2A;

  std::memset(vram_buf, 0x55, kCanarySize);
  std::memset(dst, 0x00, kDataSize);
  std::memset(dst + kDataSize, 0xAA, kCanarySize);
  __asm__ volatile("sfence" : : : "memory");

  M2MFFillPattern(src, kDataSize, kSeed);

  bool pass = true;
  if (!gpum_copy(dst, src, kDataSize)) {
    TEST_FAIL("Sysmem -> VRAM upload timed out or failed.");
    pass = false;
  } else if (!M2MFVerifyPattern(dst, kDataSize, kSeed)) {
    TEST_FAIL("Sysmem -> VRAM upload data mismatch.");
    pass = false;
  } else if (!M2MFVerifyCanaries(vram_buf, kCanarySize, 0x55) ||
             !M2MFVerifyCanaries(dst + kDataSize, kCanarySize, 0xAA)) {
    TEST_FAIL("Sysmem -> VRAM upload corrupted canaries.");
    pass = false;
  } else {
    TEST_PASS("Sysmem (WB) -> VRAM (WC) upload (16 KB) verified.");
  }

  MmFreeContiguousMemory(sysmem_buf);
  MmFreeContiguousMemory(vram_buf);
  return pass;
}
