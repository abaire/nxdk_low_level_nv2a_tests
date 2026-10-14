#include "test_m2mf_min_transfer.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

bool TestM2MFMinTransfer::Test() {
  if (!M2MFInit()) {
    TEST_FAIL("Failed to initialize M2MF engine.");
    return false;
  }

  constexpr size_t kCanarySize = 256;
  constexpr size_t kAllocSize = 1 + kCanarySize * 2;

  auto src_buf = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kAllocSize, 0, MAXRAM, 0, PAGE_READWRITE));
  auto dst_buf = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kAllocSize, 0, MAXRAM, 0, PAGE_READWRITE));

  if (!src_buf || !dst_buf) {
    TEST_FAIL("Failed to allocate test memory buffers.");
    if (src_buf) MmFreeContiguousMemory(src_buf);
    if (dst_buf) MmFreeContiguousMemory(dst_buf);
    return false;
  }

  uint8_t* src = src_buf + kCanarySize;
  uint8_t* dst = dst_buf + kCanarySize;

  std::memset(dst_buf, 0x55, kCanarySize);
  *dst = 0x00;
  std::memset(dst + 1, 0xAA, kCanarySize);
  *src = 0x7E;

  bool pass = true;
  if (gpum_copy_pitched(dst, src, 1, 1, 1, 1) == nullptr) {
    TEST_FAIL("1x1 byte transfer timed out or failed.");
    pass = false;
  } else if (*dst != 0x7E) {
    TEST_FAIL("1x1 byte transfer data mismatch (got 0x%02X, expected 0x7E).",
              *dst);
    pass = false;
  } else if (!M2MFVerifyCanaries(dst_buf, kCanarySize, 0x55) ||
             !M2MFVerifyCanaries(dst + 1, kCanarySize, 0xAA)) {
    TEST_FAIL("1x1 byte transfer corrupted boundary canaries.");
    pass = false;
  } else {
    TEST_PASS("Minimum transfer (1 byte) succeeded.");
  }

  MmFreeContiguousMemory(src_buf);
  MmFreeContiguousMemory(dst_buf);
  return pass;
}
