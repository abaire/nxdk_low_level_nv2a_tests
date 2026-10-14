#include "test_m2mf_zero_size.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

bool TestM2MFZeroSize::Test() {
  if (!M2MFInit()) {
    TEST_FAIL("Failed to initialize M2MF engine.");
    return false;
  }

  constexpr size_t kBufSize = 1024;
  auto src_buf = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kBufSize, 0, MAXRAM, 0, PAGE_READWRITE));
  auto dst_buf = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kBufSize, 0, MAXRAM, 0, PAGE_READWRITE));

  if (!src_buf || !dst_buf) {
    TEST_FAIL("Failed to allocate test memory buffers.");
    if (src_buf) MmFreeContiguousMemory(src_buf);
    if (dst_buf) MmFreeContiguousMemory(dst_buf);
    return false;
  }

  bool pass = true;

  // Zero line count
  std::memset(dst_buf, 0xCC, kBufSize);
  std::memset(src_buf, 0x55, kBufSize);

  bool zero_lines_ok =
      (gpum_copy_pitched(dst_buf, src_buf, 64, 0, 64, 64) != nullptr);
  bool dst_untouched = M2MFVerifyCanaries(dst_buf, kBufSize, 0xCC);

  if (!zero_lines_ok) {
    TEST_FAIL("Zero line count transfer timed out or reported failure.");
    pass = false;
  } else if (!dst_untouched) {
    TEST_FAIL("Zero line count unexpectedly modified destination memory.");
    pass = false;
  } else {
    TEST_PASS("Zero line count preserved destination memory.");
  }

  // Zero line length
  std::memset(dst_buf, 0xCC, kBufSize);
  bool zero_len_ok =
      (gpum_copy_pitched(dst_buf, src_buf, 0, 4, 64, 64) != nullptr);
  dst_untouched = M2MFVerifyCanaries(dst_buf, kBufSize, 0xCC);

  if (!zero_len_ok) {
    TEST_FAIL("Zero line length transfer timed out or reported failure.");
    pass = false;
  } else if (!dst_untouched) {
    TEST_FAIL("Zero line length unexpectedly modified destination memory.");
    pass = false;
  } else {
    TEST_PASS("Zero line length preserved destination memory.");
  }

  MmFreeContiguousMemory(src_buf);
  MmFreeContiguousMemory(dst_buf);
  return pass;
}
