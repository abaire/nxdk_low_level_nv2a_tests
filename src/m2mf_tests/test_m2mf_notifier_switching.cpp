#include "test_m2mf_notifier_switching.h"

#include <pbkit/pbkit.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

bool TestM2MFNotifierSwitching::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF Notifier Context Switching\n");

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

  uint32_t src_pa = M2MFGetPhysicalAddress(src_buf);
  uint32_t dst_pa = M2MFGetPhysicalAddress(dst_buf);

  // Arm initial live notifier state with a dummy copy
  if (gpum_copy(dst_buf, src_buf, 256) == nullptr) {
    LogMsg("  [FAIL] Initial gpum_copy failed.\n");
    MmFreeContiguousMemory(src_buf);
    MmFreeContiguousMemory(dst_buf);
    return false;
  }

  gpum_notifier_t prev_rec = {0};
  gpum_notifier_dump(&prev_rec);

  // Kick transfer targeting scratch notifier
  __asm__ volatile("wbinvd" : : : "memory");
  using PBKitPlusPlus::Pushbuffer;
  Pushbuffer::Begin();
  constexpr uint32_t kTransferLen = 128;
  constexpr uint32_t kTransferLines = 1;
  Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_DMA_NOTIFY,
                     kM2MDmaNotifyScratchChannel);
  Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_OFFSET_IN,
                     src_pa, dst_pa);
  Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_PITCH_IN,
                     kTransferLen, kTransferLen, kTransferLen, kTransferLines);
  Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_FORMAT,
                     kM2MFFormatLinearByteStream, kM2MFBufferNotifyKick);
  Pushbuffer::End();

  PBKitBusyWait();
  __asm__ volatile("wbinvd" : : : "memory");

  gpum_notifier_t scratch_rec = {0};
  gpum_notifier_dump(&scratch_rec);

  bool pass = true;
  if (scratch_rec.ptimer_low != prev_rec.ptimer_low ||
      scratch_rec.ptimer_high != prev_rec.ptimer_high ||
      scratch_rec.status != prev_rec.status) {
    LogMsg("  [FAIL] Live notifier was modified during scratch transfer.\n");
    pass = false;
  } else {
    LogMsg(
        "  [PASS] Method 0x180 directed completion exclusively to "
        "scratch.\n");
  }

  // Now switch back to live notifier and verify it updates
  Pushbuffer::Begin();
  Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_DMA_NOTIFY,
                     kM2MDmaNotifyChannel);
  Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_OFFSET_IN,
                     src_pa, dst_pa);
  Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_PITCH_IN,
                     kTransferLen, kTransferLen, kTransferLen, kTransferLines);
  Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_FORMAT,
                     kM2MFFormatLinearByteStream, kM2MFBufferNotifyKick);
  Pushbuffer::End();

  PBKitBusyWait();
  __asm__ volatile("wbinvd" : : : "memory");

  gpum_notifier_t live_rec = {0};
  gpum_notifier_dump(&live_rec);

  if (live_rec.status != 0) {
    LogMsg("  [FAIL] Live notifier status is not 0 (0x%08X).\n",
           live_rec.status);
    pass = false;
  } else if (live_rec.ptimer_low == prev_rec.ptimer_low &&
             live_rec.ptimer_high == prev_rec.ptimer_high) {
    LogMsg("  [FAIL] Live notifier timestamp did not advance.\n");
    pass = false;
  } else {
    LogMsg("  [PASS] Method 0x180 switched back cleanly to live notifier.\n");
  }

  MmFreeContiguousMemory(src_buf);
  MmFreeContiguousMemory(dst_buf);
  return pass;
}
