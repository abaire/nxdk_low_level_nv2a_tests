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
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF Framebuffer Aperture Write\n");

  if (!M2MFInit()) {
    LogMsg("  [FAIL] Failed to initialize M2MF engine.\n");
    return false;
  }

  if (pb_FBAddr[pb_back_index] == 0) {
    LogMsg("  [SKIP] Framebuffer backbuffer address is not available.\n");
    return true;
  }

  constexpr size_t kCanarySize = 256;
  constexpr size_t kFbCopySize = 4096;
  constexpr size_t kAllocSize = kFbCopySize + kCanarySize * 2;

  auto sysmem_buf = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kAllocSize, 0, MAXRAM, 0, PAGE_READWRITE));

  if (!sysmem_buf) {
    LogMsg("  [FAIL] Failed to allocate test memory buffer.\n");
    return false;
  }

  uint8_t* fb = reinterpret_cast<uint8_t*>(pb_FBAddr[pb_back_index]);
  uint8_t* src = sysmem_buf + kCanarySize;
  constexpr uint8_t kSeed = 0x77;

  M2MFFillPattern(src, kFbCopySize, kSeed);

  bool pass = true;
  if (!gpum_copy(fb, src, kFbCopySize)) {
    LogMsg("  [FAIL] Sysmem -> Framebuffer transfer timed out or failed.\n");
    pass = false;
  } else if (!M2MFVerifyPattern(fb, kFbCopySize, kSeed)) {
    LogMsg("  [FAIL] Sysmem -> Framebuffer transfer data mismatch.\n");
    pass = false;
  } else {
    LogMsg("  [PASS] Sysmem -> Active Framebuffer aperture (4 KB) verified.\n");
  }

  MmFreeContiguousMemory(sysmem_buf);
  return pass;
}
