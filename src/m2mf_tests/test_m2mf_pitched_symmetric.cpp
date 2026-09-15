#include "test_m2mf_pitched_symmetric.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

bool TestM2MFPitchedSymmetric::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF Pitched 2D Symmetric\n");

  if (!M2MFInit()) {
    LogMsg("  [FAIL] Failed to initialize M2MF engine.\n");
    return false;
  }

  constexpr size_t kCanarySize = 256;
  constexpr uint32_t kLineLen = 64;
  constexpr uint32_t kLines = 8;
  constexpr uint32_t kPitch = 128;
  constexpr size_t kTotalData = kPitch * kLines;
  constexpr size_t kAllocSize = kTotalData + kCanarySize * 2;

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
  constexpr uint8_t kSeed = 0x3C;

  std::memset(dst_buf, 0x55, kCanarySize);
  std::memset(dst, 0xEE, kTotalData);
  std::memset(dst + kTotalData, 0xAA, kCanarySize);

  M2MFFillPattern(src, kTotalData, kSeed);

  bool pass = true;
  void* ret = gpum_copy_pitched(dst, src, kLineLen, kLines, kPitch, kPitch);
  if (!ret) {
    LogMsg("  [FAIL] Symmetric pitched transfer timed out or failed.\n");
    pass = false;
  } else {
    bool data_ok = true;
    for (uint32_t y = 0; y < kLines; ++y) {
      const uint8_t* s_line = src + y * kPitch;
      const uint8_t* d_line = dst + y * kPitch;

      if (std::memcmp(d_line, s_line, kLineLen) != 0) {
        LogMsg("  [FAIL] Line %u data mismatch.\n", y);
        data_ok = false;
        break;
      }

      const uint8_t* gap = d_line + kLineLen;
      size_t gap_len = kPitch - kLineLen;
      if (!M2MFVerifyCanaries(gap, gap_len, 0xEE)) {
        LogMsg("  [FAIL] Line %u inter-line gap corrupted.\n", y);
        data_ok = false;
        break;
      }
    }

    if (!data_ok) {
      pass = false;
    } else if (!M2MFVerifyCanaries(dst_buf, kCanarySize, 0x55) ||
               !M2MFVerifyCanaries(dst + kTotalData, kCanarySize, 0xAA)) {
      LogMsg("  [FAIL] Outer boundary canaries corrupted.\n");
      pass = false;
    } else {
      LogMsg(
          "  [PASS] Symmetric pitched transfer (len=%u, pitch=%u, lines=%u) "
          "verified.\n",
          kLineLen, kPitch, kLines);
    }
  }

  MmFreeContiguousMemory(src_buf);
  MmFreeContiguousMemory(dst_buf);
  return pass;
}
