#include "test_m2mf_alignment_phases.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

bool TestM2MFAlignmentPhases::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF Alignment Phase Matrix\n");

  if (!M2MFInit()) {
    LogMsg("  [FAIL] Failed to initialize M2MF engine.\n");
    return false;
  }

  constexpr size_t kCanarySize = 128;
  constexpr size_t kMaxTransfer = 2048;
  constexpr size_t kAllocSize = kMaxTransfer + kCanarySize * 2 + 16;

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

  constexpr size_t kPhaseTestLen = 127;
  bool pass = true;

  for (size_t s_off = 0; s_off < 4; ++s_off) {
    for (size_t d_off = 0; d_off < 4; ++d_off) {
      uint8_t* src = src_buf + kCanarySize + s_off;
      uint8_t* dst = dst_buf + kCanarySize + d_off;
      uint8_t seed = static_cast<uint8_t>((s_off << 4) | d_off);

      std::memset(dst_buf, 0x55, kCanarySize + d_off);
      std::memset(dst, 0xCC, kPhaseTestLen);
      std::memset(dst + kPhaseTestLen, 0xAA, kCanarySize);

      M2MFFillPattern(src, kPhaseTestLen, seed);

      if (!gpum_copy(dst, src, kPhaseTestLen)) {
        LogMsg(
            "  [FAIL] Phase src+%u -> dst+%u: Transfer failed or timed out.\n",
            s_off, d_off);
        pass = false;
        continue;
      }

      if (!M2MFVerifyPattern(dst, kPhaseTestLen, seed)) {
        LogMsg("  [FAIL] Phase src+%u -> dst+%u: Data mismatch.\n", s_off,
               d_off);
        pass = false;
      } else if (!M2MFVerifyCanaries(dst_buf, kCanarySize + d_off, 0x55)) {
        LogMsg("  [FAIL] Phase src+%u -> dst+%u: Pre-guard canary corrupted.\n",
               s_off, d_off);
        pass = false;
      } else if (!M2MFVerifyCanaries(dst + kPhaseTestLen, kCanarySize, 0xAA)) {
        LogMsg(
            "  [FAIL] Phase src+%u -> dst+%u: Post-guard canary corrupted.\n",
            s_off, d_off);
        pass = false;
      }
    }
  }

  if (pass) {
    LogMsg("  [PASS] Verified all 16 (src %% 4, dst %% 4) alignment phases.\n");
  }

  MmFreeContiguousMemory(src_buf);
  MmFreeContiguousMemory(dst_buf);
  return pass;
}
