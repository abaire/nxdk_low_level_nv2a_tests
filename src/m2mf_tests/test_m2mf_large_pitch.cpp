#include "test_m2mf_large_pitch.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

bool TestM2MFLargePitch::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF Large Pitch Stride (16 KB)\n");

  if (!M2MFInit()) {
    LogMsg("  [FAIL] Failed to initialize M2MF engine.\n");
    return false;
  }

  constexpr size_t kCanarySize = 256;
  constexpr uint32_t kLargePitch = 16384;
  constexpr uint32_t kPitchedLines = 4;
  constexpr uint32_t kPitchedLen = 64;
  constexpr size_t kTotalSpan = kLargePitch * kPitchedLines;
  constexpr size_t kAllocSize = kTotalSpan + kCanarySize * 2;

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

  std::memset(dst_buf, 0x55, kCanarySize);
  std::memset(dst, 0xEE, kTotalSpan);
  std::memset(dst + kTotalSpan, 0xAA, kCanarySize);
  M2MFFillPattern(src, kTotalSpan, 0x81);

  bool pass = true;
  if (gpum_copy_pitched(dst, src, kPitchedLen, kPitchedLines, kLargePitch,
                        kLargePitch) == nullptr) {
    LogMsg("  [FAIL] Large pitch (16384 B) transfer timed out or failed.\n");
    pass = false;
  } else {
    bool pitched_ok = true;
    for (uint32_t y = 0; y < kPitchedLines; ++y) {
      if (std::memcmp(dst + y * kLargePitch, src + y * kLargePitch,
                      kPitchedLen) != 0) {
        pitched_ok = false;
        break;
      }
      if (!M2MFVerifyCanaries(dst + y * kLargePitch + kPitchedLen,
                              kLargePitch - kPitchedLen, 0xEE)) {
        pitched_ok = false;
        break;
      }
    }
    if (!pitched_ok) {
      LogMsg("  [FAIL] Large pitch transfer data or gap mismatch.\n");
      pass = false;
    } else if (!M2MFVerifyCanaries(dst_buf, kCanarySize, 0x55) ||
               !M2MFVerifyCanaries(dst + kTotalSpan, kCanarySize, 0xAA)) {
      LogMsg("  [FAIL] Large pitch corrupted outer guard canaries.\n");
      pass = false;
    } else {
      LogMsg("  [PASS] Large pitch (16384 B stride) succeeded.\n");
    }
  }

  MmFreeContiguousMemory(src_buf);
  MmFreeContiguousMemory(dst_buf);
  return pass;
}
