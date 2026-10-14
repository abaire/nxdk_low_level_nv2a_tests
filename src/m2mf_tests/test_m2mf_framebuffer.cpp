#include "test_m2mf_framebuffer.h"

#include <pbkit/pbkit.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

extern "C" {
extern DWORD pb_FBAddr[3];
extern int pb_back_index;
}

bool TestM2MFFramebuffer::Test() {
  if (!M2MFInit()) {
    TEST_FAIL("Failed to initialize M2MF engine.");
    return false;
  }

  if (pb_FBAddr[pb_back_index] == 0) {
    TEST_INFO("[SKIP] Framebuffer backbuffer address is not available.");
    return true;
  }

  constexpr size_t kCanarySize = 256;
  constexpr size_t kFbCopySize = 4096;
  constexpr size_t kAllocSize = kFbCopySize + kCanarySize * 2;

  auto sysmem_buf = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kAllocSize, 0, MAXRAM, 0, PAGE_READWRITE));

  if (!sysmem_buf) {
    TEST_FAIL("Failed to allocate test memory buffer.");
    return false;
  }

  uint8_t* fb = reinterpret_cast<uint8_t*>(pb_FBAddr[pb_back_index]);
  uint8_t* src = sysmem_buf + kCanarySize;
  constexpr uint8_t kSeed = 0x77;

  M2MFFillPattern(src, kFbCopySize, kSeed);

  bool pass = true;
  if (!gpum_copy(fb, src, kFbCopySize)) {
    TEST_FAIL("Sysmem -> Framebuffer transfer timed out or failed.");
    pass = false;
  } else if (!M2MFVerifyPattern(fb, kFbCopySize, kSeed)) {
    TEST_FAIL("Sysmem -> Framebuffer transfer data mismatch.");
    pass = false;
  } else {
    TEST_PASS("Sysmem -> Active Framebuffer aperture (4 KB) verified.");
  }

  MmFreeContiguousMemory(sysmem_buf);
  return pass;
}
