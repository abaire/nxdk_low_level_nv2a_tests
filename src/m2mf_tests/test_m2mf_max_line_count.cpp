#include "test_m2mf_max_line_count.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

bool TestM2MFMaxLineCount::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF Max Line Count (2047)\n");

  if (!M2MFInit()) {
    LogMsg("  [FAIL] Failed to initialize M2MF engine.\n");
    return false;
  }

  constexpr size_t kCanarySize = 256;
  constexpr size_t kMaxLines = 2047;
  constexpr size_t kLineLen = 32;
  constexpr size_t kTotalDataSize = kMaxLines * kLineLen;
  constexpr size_t kAllocSize = kTotalDataSize + kCanarySize * 2;

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
  constexpr uint8_t kSeed = 0x4D;

  std::memset(dst_buf, 0x55, kCanarySize);
  std::memset(dst, 0xCC, kTotalDataSize);
  std::memset(dst + kTotalDataSize, 0xAA, kCanarySize);

  M2MFFillPattern(src, kTotalDataSize, kSeed);

  bool pass = true;
  if (gpum_copy_pitched(dst, src, kLineLen, kMaxLines, kLineLen, kLineLen) ==
      nullptr) {
    LogMsg("  [FAIL] Maximum line count (2047 lines) timed out or failed.\n");
    pass = false;
  } else if (!M2MFVerifyPattern(dst, kTotalDataSize, kSeed)) {
    LogMsg("  [FAIL] Maximum line count (2047 lines) data mismatch.\n");
    pass = false;
  } else if (!M2MFVerifyCanaries(dst_buf, kCanarySize, 0x55) ||
             !M2MFVerifyCanaries(dst + kTotalDataSize, kCanarySize, 0xAA)) {
    LogMsg("  [FAIL] Maximum line count corrupted boundary canaries.\n");
    pass = false;
  } else {
    LogMsg(
        "  [PASS] Maximum line count (2047 lines x 32 B = %u B) succeeded.\n",
        kTotalDataSize);
  }

  MmFreeContiguousMemory(src_buf);
  MmFreeContiguousMemory(dst_buf);
  return pass;
}
