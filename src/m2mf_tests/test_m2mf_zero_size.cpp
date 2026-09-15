#include "test_m2mf_zero_size.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

bool TestM2MFZeroSize::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF Zero Count / Length\n");

  if (!M2MFInit()) {
    LogMsg("  [FAIL] Failed to initialize M2MF engine.\n");
    return false;
  }

  constexpr size_t kBufSize = 1024;
  auto src_buf = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kBufSize, 0, MAXRAM, 0, PAGE_READWRITE));
  auto dst_buf = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kBufSize, 0, MAXRAM, 0, PAGE_READWRITE));

  if (!src_buf || !dst_buf) {
    LogMsg("  [FAIL] Failed to allocate test memory buffers.\n");
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
    LogMsg(
        "  [FAIL] Zero line count transfer timed out or reported failure.\n");
    pass = false;
  } else if (!dst_untouched) {
    LogMsg(
        "  [FAIL] Zero line count unexpectedly modified destination memory.\n");
    pass = false;
  } else {
    LogMsg("  [PASS] Zero line count preserved destination memory.\n");
  }

  // Zero line length
  std::memset(dst_buf, 0xCC, kBufSize);
  bool zero_len_ok =
      (gpum_copy_pitched(dst_buf, src_buf, 0, 4, 64, 64) != nullptr);
  dst_untouched = M2MFVerifyCanaries(dst_buf, kBufSize, 0xCC);

  if (!zero_len_ok) {
    LogMsg(
        "  [FAIL] Zero line length transfer timed out or reported failure.\n");
    pass = false;
  } else if (!dst_untouched) {
    LogMsg(
        "  [FAIL] Zero line length unexpectedly modified destination "
        "memory.\n");
    pass = false;
  } else {
    LogMsg("  [PASS] Zero line length preserved destination memory.\n");
  }

  MmFreeContiguousMemory(src_buf);
  MmFreeContiguousMemory(dst_buf);
  return pass;
}
