#include "test_m2mf_alignment_odd_sizes.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

struct OddSizeCase {
  size_t size;
  size_t src_align;
  size_t dst_align;
};

bool TestM2MFAlignmentOddSizes::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF Alignment Odd & Prime Sizes\n");

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

  const OddSizeCase odd_cases[] = {
      {1, 0, 0}, {1, 1, 3},  {1, 3, 1},  {2, 1, 2},  {3, 2, 1},    {5, 3, 2},
      {7, 1, 3}, {13, 2, 3}, {31, 3, 1}, {63, 1, 2}, {1027, 3, 2},
  };

  bool pass = true;
  for (const auto& oc : odd_cases) {
    uint8_t* src = src_buf + kCanarySize + oc.src_align;
    uint8_t* dst = dst_buf + kCanarySize + oc.dst_align;
    uint8_t seed = static_cast<uint8_t>(oc.size ^ 0x9B);

    std::memset(dst_buf, 0x55, kCanarySize + oc.dst_align);
    std::memset(dst, 0xCC, oc.size);
    std::memset(dst + oc.size, 0xAA, kCanarySize);

    M2MFFillPattern(src, oc.size, seed);

    if (!gpum_copy(dst, src, oc.size)) {
      LogMsg(
          "  [FAIL] Odd size %u (s+%u, d+%u): Transfer failed or timed out.\n",
          oc.size, oc.src_align, oc.dst_align);
      pass = false;
      continue;
    }

    if (!M2MFVerifyPattern(dst, oc.size, seed)) {
      LogMsg("  [FAIL] Odd size %u (s+%u, d+%u): Data mismatch.\n", oc.size,
             oc.src_align, oc.dst_align);
      pass = false;
    } else if (!M2MFVerifyCanaries(dst_buf, kCanarySize + oc.dst_align, 0x55)) {
      LogMsg("  [FAIL] Odd size %u (s+%u, d+%u): Pre-canary corrupted.\n",
             oc.size, oc.src_align, oc.dst_align);
      pass = false;
    } else if (!M2MFVerifyCanaries(dst + oc.size, kCanarySize, 0xAA)) {
      LogMsg("  [FAIL] Odd size %u (s+%u, d+%u): Post-canary corrupted.\n",
             oc.size, oc.src_align, oc.dst_align);
      pass = false;
    }
  }

  if (pass) {
    LogMsg(
        "  [PASS] Verified non-power-of-two and odd byte sizes (1..1027 B).\n");
  }

  MmFreeContiguousMemory(src_buf);
  MmFreeContiguousMemory(dst_buf);
  return pass;
}
