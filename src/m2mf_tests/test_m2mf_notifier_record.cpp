#include "test_m2mf_notifier_record.h"

#include <pbkit/pbkit.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

#define NV_PTIMER_TIME_0 0x00009400
#define NV_PTIMER_TIME_1 0x00009410

bool TestM2MFNotifierRecord::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF Notifier Completion Record\n");

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

  std::memset(src_buf, 0x33, kBufSize);
  std::memset(dst_buf, 0x00, kBufSize);

  uint32_t ptimer_before_lo = VIDEOREG(NV_PTIMER_TIME_0);
  uint32_t ptimer_before_hi = VIDEOREG(NV_PTIMER_TIME_1);

  bool pass = true;
  if (gpum_copy(dst_buf, src_buf, 256) == nullptr) {
    LogMsg("  [FAIL] gpum_copy failed.\n");
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

    LogMsg("  Live notifier completion record:\n");
    LogMsg("    PTIMER_LO: 0x%08X (sample range: 0x%08X .. 0x%08X)\n", n_lo,
           ptimer_before_lo, ptimer_after_lo);
    LogMsg("    PTIMER_HI: 0x%08X (sample range: 0x%08X .. 0x%08X)\n", n_hi,
           ptimer_before_hi, ptimer_after_hi);
    LogMsg("    ERROR    : 0x%08X\n", n_err);
    LogMsg("    STATUS   : 0x%08X\n", n_stat);

    if (n_stat != 0) {
      LogMsg("  [FAIL] Expected status 0, got 0x%08X.\n", n_stat);
      pass = false;
    } else if (n_err != 0) {
      LogMsg("  [FAIL] Expected error 0, got 0x%08X.\n", n_err);
      pass = false;
    } else if (n_lo == 0 && n_hi == 0) {
      LogMsg("  [FAIL] PTIMER timestamp was not written by hardware.\n");
      pass = false;
    } else {
      LogMsg("  [PASS] Completion record format and status validated.\n");
    }
  }

  MmFreeContiguousMemory(src_buf);
  MmFreeContiguousMemory(dst_buf);
  return pass;
}
