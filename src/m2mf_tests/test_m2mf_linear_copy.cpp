#include "test_m2mf_linear_copy.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"
#include "third_party/gpu_m2m.h"

bool TestM2MFLinearCopy::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF Linear Copy\n");

  if (!M2MFInit()) {
    LogMsg("  [FAIL] Failed to initialize M2MF engine.\n");
    return false;
  }

  constexpr size_t kCanarySize = 256;
  constexpr size_t kCopySize = 4096;
  constexpr size_t kAllocSize = kCopySize + kCanarySize * 2;

  auto src_buf = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kAllocSize, 0, MAXRAM, 0, PAGE_READWRITE));
  auto dst_buf = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kAllocSize, 0, MAXRAM, 0, PAGE_READWRITE));

  if (!src_buf || !dst_buf) {
    LogMsg("  [FAIL] Failed to allocate test memory buffers.\n");
    if (src_buf) MmFreeContiguousMemory(src_buf);
    if (dst_buf) MmFreeContiguousMemory(dst_buf);
    return false;
  }

  uint8_t* src = src_buf + kCanarySize;
  uint8_t* dst = dst_buf + kCanarySize;
  constexpr uint8_t kSeed = 0x5A;

  std::memset(dst_buf, 0x55, kCanarySize);
  std::memset(dst, 0xCC, kCopySize);
  std::memset(dst + kCopySize, 0xAA, kCanarySize);

  M2MFFillPattern(src, kCopySize, kSeed);

  LogMsg("  Initiating gpum_copy (%u bytes)...\n", kCopySize);
  void* ret = gpum_copy(dst, src, kCopySize);
  if (!ret) {
    gpum_notifier_t notif = {0};
    gpum_notifier_dump(&notif);
    LogMsg(
        "  [FAIL] gpum_copy failed. Notifier: lo=0x%08X hi=0x%08X err=0x%08X "
        "st=0x%08X\n",
        notif.ptimer_low, notif.ptimer_high, notif.error, notif.status);
    MmFreeContiguousMemory(src_buf);
    MmFreeContiguousMemory(dst_buf);
    return false;
  }

  bool pass = true;
  if (!M2MFVerifyPattern(dst, kCopySize, kSeed)) {
    LogMsg("  [FAIL] Destination content does not match source pattern.\n");
    pass = false;
  } else if (!M2MFVerifyCanaries(dst_buf, kCanarySize, 0x55)) {
    LogMsg("  [FAIL] Pre-guard canary corrupted.\n");
    pass = false;
  } else if (!M2MFVerifyCanaries(dst + kCopySize, kCanarySize, 0xAA)) {
    LogMsg("  [FAIL] Post-guard canary corrupted.\n");
    pass = false;
  } else if (!M2MFVerifyPattern(src, kCopySize, kSeed)) {
    LogMsg("  [FAIL] Source buffer unexpectedly modified.\n");
    pass = false;
  } else {
    LogMsg(
        "  [PASS] Verified 4096 B copy: destination matches source, canaries "
        "intact.\n");
  }

  MmFreeContiguousMemory(src_buf);
  MmFreeContiguousMemory(dst_buf);
  return pass;
}
