#include "test_m2mf_notifier_record.h"

#include <pbkit/pbkit.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

#define NV_PTIMER_TIME_0 0x00009400
#define NV_PTIMER_TIME_1 0x00009410

bool TestM2MFNotifierRecord::Test() {
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

  std::memset(src_buf, 0x33, kBufSize);
  std::memset(dst_buf, 0x00, kBufSize);

  uint32_t ptimer_before_lo = VIDEOREG(NV_PTIMER_TIME_0);
  uint32_t ptimer_before_hi = VIDEOREG(NV_PTIMER_TIME_1);

  bool pass = true;
  if (gpum_copy(dst_buf, src_buf, 256) == nullptr) {
    TEST_FAIL("gpum_copy failed.");
    pass = false;
  } else {
    uint32_t ptimer_after_lo = VIDEOREG(NV_PTIMER_TIME_0);
    uint32_t ptimer_after_hi = VIDEOREG(NV_PTIMER_TIME_1);

    gpum_notifier_t rec = {0};
    gpum_notifier_dump(&rec);
    uint32_t n_lo = rec.ptimer_low;
    uint32_t n_hi = rec.ptimer_high;
    uint32_t n_err = rec.error;
    uint32_t n_stat = rec.status;

    TEST_INFO("Live notifier completion record:");
    TEST_INFO("  PTIMER_LO: 0x%08X (sample range: 0x%08X .. 0x%08X)", n_lo,
              ptimer_before_lo, ptimer_after_lo);
    TEST_INFO("  PTIMER_HI: 0x%08X (sample range: 0x%08X .. 0x%08X)", n_hi,
              ptimer_before_hi, ptimer_after_hi);
    TEST_INFO("  ERROR    : 0x%08X", n_err);
    TEST_INFO("  STATUS   : 0x%08X", n_stat);

    if (n_stat != 0) {
      TEST_FAIL_EX(TestFields("expected", Hex32(0), "actual", Hex32(n_stat)),
                   "Expected status 0, got 0x%08X.", n_stat);
      pass = false;
    } else if (n_err != 0) {
      TEST_FAIL("Expected error 0, got 0x%08X.", n_err);
      pass = false;
    } else if (n_lo == 0 && n_hi == 0) {
      TEST_FAIL("PTIMER timestamp was not written by hardware.");
      pass = false;
    } else {
      TEST_PASS("Completion record format and status validated.");
    }
  }

  MmFreeContiguousMemory(src_buf);
  MmFreeContiguousMemory(dst_buf);
  return pass;
}
