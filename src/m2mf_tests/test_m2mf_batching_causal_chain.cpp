#include "test_m2mf_batching_causal_chain.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

bool TestM2MFBatchingCausalChain::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF Causal Chained Transfers\n");

  if (!M2MFInit()) {
    LogMsg("  [FAIL] Failed to initialize M2MF engine.\n");
    return false;
  }

  constexpr size_t kChainSize = 1024;
  auto buf_a = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kChainSize, 0, MAXRAM, 0, PAGE_READWRITE));
  auto buf_b = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kChainSize, 0, MAXRAM, 0, PAGE_READWRITE));
  auto buf_c = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kChainSize, 0, MAXRAM, 0, PAGE_READWRITE));

  if (!buf_a || !buf_b || !buf_c) {
    LogMsg("  [FAIL] Memory allocation failed for chained transfer test.\n");
    if (buf_a) MmFreeContiguousMemory(buf_a);
    if (buf_b) MmFreeContiguousMemory(buf_b);
    if (buf_c) MmFreeContiguousMemory(buf_c);
    return false;
  }

  constexpr uint8_t kSeed = 0x83;
  M2MFFillPattern(buf_a, kChainSize, kSeed);
  std::memset(buf_b, 0x11, kChainSize);
  std::memset(buf_c, 0x22, kChainSize);

  uint32_t a_pa = M2MFGetPhysicalAddress(buf_a);
  uint32_t b_pa = M2MFGetPhysicalAddress(buf_b);
  uint32_t c_pa = M2MFGetPhysicalAddress(buf_c);

  __asm__ volatile("wbinvd" : : : "memory");

  using PBKitPlusPlus::Pushbuffer;
  Pushbuffer::Begin();
  // Step 1: A -> B
  Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_OFFSET_IN, a_pa,
                     b_pa);
  Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_PITCH_IN,
                     static_cast<uint32_t>(kChainSize),
                     static_cast<uint32_t>(kChainSize),
                     static_cast<uint32_t>(kChainSize), 1u);
  Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_FORMAT,
                     kM2MFFormatLinearByteStream, kM2MFBufferNotifyKick);

  // Step 2: B -> C
  Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_OFFSET_IN, b_pa,
                     c_pa);
  Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_PITCH_IN,
                     static_cast<uint32_t>(kChainSize),
                     static_cast<uint32_t>(kChainSize),
                     static_cast<uint32_t>(kChainSize), 1u);
  Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_FORMAT,
                     kM2MFFormatLinearByteStream, kM2MFBufferNotifyKick);
  Pushbuffer::End();

  PBKitBusyWait();
  __asm__ volatile("wbinvd" : : : "memory");

  bool b_ok = M2MFVerifyPattern(buf_b, kChainSize, kSeed);
  bool c_ok = M2MFVerifyPattern(buf_c, kChainSize, kSeed);

  bool pass = true;
  if (!b_ok) {
    LogMsg(
        "  [FAIL] Intermediate Buffer B did not receive expected data from "
        "A.\n");
    pass = false;
  } else if (!c_ok) {
    LogMsg(
        "  [FAIL] Final Buffer C did not receive chained data from B "
        "(hazard).\n");
    pass = false;
  } else {
    LogMsg(
        "  [PASS] Chained transfer (A -> B -> C, %u B) executed hazard-free.\n",
        kChainSize);
  }

  MmFreeContiguousMemory(buf_a);
  MmFreeContiguousMemory(buf_b);
  MmFreeContiguousMemory(buf_c);
  return pass;
}
