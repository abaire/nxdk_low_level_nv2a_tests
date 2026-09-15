#include "test_m2mf_batching_independent.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

bool TestM2MFBatchingIndependent::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF Batched Independent Transfers\n");

  if (!M2MFInit()) {
    LogMsg("  [FAIL] Failed to initialize M2MF engine.\n");
    return false;
  }

  constexpr size_t kNumChunks = 8;
  constexpr size_t kChunkSize = 512;
  constexpr size_t kTotalSize = kNumChunks * kChunkSize;

  auto src_mem = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kTotalSize, 0, MAXRAM, 0, PAGE_READWRITE));
  auto dst_mem = static_cast<uint8_t*>(
      MmAllocateContiguousMemoryEx(kTotalSize, 0, MAXRAM, 0, PAGE_READWRITE));

  if (!src_mem || !dst_mem) {
    LogMsg("  [FAIL] Memory allocation failed for independent batch test.\n");
    if (src_mem) MmFreeContiguousMemory(src_mem);
    if (dst_mem) MmFreeContiguousMemory(dst_mem);
    return false;
  }

  std::memset(dst_mem, 0xCC, kTotalSize);
  for (size_t i = 0; i < kNumChunks; ++i) {
    M2MFFillPattern(src_mem + i * kChunkSize, kChunkSize,
                    static_cast<uint8_t>(0x10 * i + 7));
  }

  __asm__ volatile("wbinvd" : : : "memory");

  using PBKitPlusPlus::Pushbuffer;
  Pushbuffer::Begin();
  for (size_t i = 0; i < kNumChunks; ++i) {
    uint32_t s_pa = M2MFGetPhysicalAddress(src_mem + i * kChunkSize);
    uint32_t d_pa = M2MFGetPhysicalAddress(dst_mem + i * kChunkSize);

    Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_OFFSET_IN,
                       s_pa, d_pa);
    Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_PITCH_IN,
                       static_cast<uint32_t>(kChunkSize),
                       static_cast<uint32_t>(kChunkSize),
                       static_cast<uint32_t>(kChunkSize), 1u);
    Pushbuffer::PushTo(kM2MSubchannel, NV_MEMORY_TO_MEMORY_FORMAT_FORMAT,
                       kM2MFFormatLinearByteStream, kM2MFBufferNotifyKick);
  }
  Pushbuffer::End();

  PBKitBusyWait();
  __asm__ volatile("wbinvd" : : : "memory");

  bool pass = true;
  for (size_t i = 0; i < kNumChunks; ++i) {
    if (!M2MFVerifyPattern(dst_mem + i * kChunkSize, kChunkSize,
                           static_cast<uint8_t>(0x10 * i + 7))) {
      LogMsg("  [FAIL] Batched transfer chunk %u data mismatch.\n", i);
      pass = false;
      break;
    }
  }

  if (pass) {
    LogMsg(
        "  [PASS] Batched independent transfers (%u chunks x %u B) "
        "verified.\n",
        kNumChunks, kChunkSize);
  }

  MmFreeContiguousMemory(src_mem);
  MmFreeContiguousMemory(dst_mem);
  return pass;
}
