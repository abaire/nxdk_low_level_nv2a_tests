#include "test_m2mf_pitched_asymmetric.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

struct AsymmetricCase {
  uint32_t line_len;
  uint32_t lines;
  uint32_t pitch_in;
  uint32_t pitch_out;
  const char* desc;
};

bool TestM2MFPitchedAsymmetric::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF Pitched 2D Asymmetric\n");

  if (!M2MFInit()) {
    LogMsg("  [FAIL] Failed to initialize M2MF engine.\n");
    return false;
  }

  constexpr size_t kCanarySize = 256;
  constexpr size_t kMaxSpan = 65536;
  constexpr size_t kAllocSize = kMaxSpan + kCanarySize * 2;

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

  const AsymmetricCase cases[] = {
      {32, 16, 256, 64, "pitch_in (256) > pitch_out (64)"},
      {48, 10, 64, 128, "pitch_in (64) < pitch_out (128)"},
  };

  bool pass = true;

  for (const auto& tc : cases) {
    uint8_t* src = src_buf + kCanarySize;
    uint8_t* dst = dst_buf + kCanarySize;
    uint8_t seed = static_cast<uint8_t>(tc.pitch_in ^ tc.pitch_out);

    size_t src_total = tc.pitch_in * tc.lines;
    size_t dst_total = tc.pitch_out * tc.lines;

    std::memset(dst_buf, 0x55, kCanarySize);
    std::memset(dst, 0xEE, dst_total);
    std::memset(dst + dst_total, 0xAA, kCanarySize);

    M2MFFillPattern(src, src_total, seed);

    void* ret = gpum_copy_pitched(dst, src, tc.line_len, tc.lines, tc.pitch_in,
                                  tc.pitch_out);
    if (!ret) {
      LogMsg("  [FAIL] %s: M2MF transfer failed or timed out.\n", tc.desc);
      pass = false;
      continue;
    }

    bool case_ok = true;
    for (uint32_t y = 0; y < tc.lines; ++y) {
      const uint8_t* s_line = src + y * tc.pitch_in;
      const uint8_t* d_line = dst + y * tc.pitch_out;

      if (std::memcmp(d_line, s_line, tc.line_len) != 0) {
        LogMsg("  [FAIL] %s: Line %u data mismatch.\n", tc.desc, y);
        case_ok = false;
        break;
      }

      if (tc.pitch_out > tc.line_len) {
        const uint8_t* gap = d_line + tc.line_len;
        size_t gap_len = tc.pitch_out - tc.line_len;
        if (!M2MFVerifyCanaries(gap, gap_len, 0xEE)) {
          LogMsg("  [FAIL] %s: Line %u gap corrupted.\n", tc.desc, y);
          case_ok = false;
          break;
        }
      }
    }

    if (case_ok) {
      if (!M2MFVerifyCanaries(dst_buf, kCanarySize, 0x55) ||
          !M2MFVerifyCanaries(dst + dst_total, kCanarySize, 0xAA)) {
        LogMsg("  [FAIL] %s: Boundary canaries corrupted.\n", tc.desc);
        case_ok = false;
      }
    }

    if (case_ok) {
      LogMsg("  [PASS] %s verified.\n", tc.desc);
    } else {
      pass = false;
    }
  }

  MmFreeContiguousMemory(src_buf);
  MmFreeContiguousMemory(dst_buf);
  return pass;
}
