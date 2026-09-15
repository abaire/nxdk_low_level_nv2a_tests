#include "test_m2mf_format.h"

#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "m2mf_test_common.h"

static bool VerifyStridedOutput(const uint8_t* dst, const uint8_t* src,
                                uint32_t line_len, uint32_t lines,
                                uint32_t pitch_in, uint32_t pitch_out,
                                uint32_t stride_in, uint32_t stride_out,
                                uint8_t canary_dst, size_t total_dst_size) {
  if (total_dst_size > 1024) {
    total_dst_size = 1024;
  }
  bool written[1024] = {false};

  for (uint32_t y = 0; y < lines; ++y) {
    for (uint32_t x = 0; x < line_len; ++x) {
      size_t dst_off = y * pitch_out + x * stride_out;
      size_t src_off = y * pitch_in + x * stride_in;
      if (dst_off >= total_dst_size) {
        LogMsg("  [FAIL] Write offset %u exceeds buffer span %u\n",
               static_cast<uint32_t>(dst_off),
               static_cast<uint32_t>(total_dst_size));
        return false;
      }
      written[dst_off] = true;
      if (dst[dst_off] != src[src_off]) {
        LogMsg(
            "  [FAIL] Value mismatch at dst[%u]: expected 0x%02X (from "
            "src[%u]), got 0x%02X\n",
            static_cast<uint32_t>(dst_off), src[src_off],
            static_cast<uint32_t>(src_off), dst[dst_off]);
        return false;
      }
    }
  }

  for (size_t i = 0; i < total_dst_size; ++i) {
    if (!written[i] && dst[i] != canary_dst) {
      LogMsg(
          "  [FAIL] Untouched gap byte dst[%u] corrupted: expected 0x%02X, got "
          "0x%02X\n",
          static_cast<uint32_t>(i), canary_dst, dst[i]);
      return false;
    }
  }

  return true;
}

bool TestM2MFFormat::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: M2MF Format Variations\n");

  if (!M2MFInit()) {
    LogMsg("  [FAIL] Failed to initialize M2MF engine.\n");
    return false;
  }

  constexpr size_t kCanarySize = 256;
  constexpr size_t kDataSize = 1024;
  constexpr size_t kAllocSize = kDataSize + kCanarySize * 2;
  constexpr uint8_t kDstCanary = 0xCC;

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

  uint8_t* src = src_buf + kCanarySize;
  uint8_t* dst = dst_buf + kCanarySize;

  auto reset_buffers = [&]() {
    std::memset(src_buf, 0x00, kAllocSize);
    std::memset(dst_buf, 0x55, kCanarySize);
    std::memset(dst, kDstCanary, kDataSize);
    std::memset(dst + kDataSize, 0xAA, kCanarySize);
  };

  bool pass = true;

  // 1. stride_in = 1, stride_out = 2 (0x0201): Single-line dispersal
  {
    reset_buffers();
    for (uint8_t i = 0; i < 16; ++i) {
      src[i] = static_cast<uint8_t>(0x10 + i);
    }

    void* ret = gpum_copy_strided(dst, src, 16, 1, 16, 32, 1, 2);
    if (!ret) {
      LogMsg("  [FAIL] 0x0201 single-line transfer timed out or failed.\n");
      pass = false;
    } else if (!VerifyStridedOutput(dst, src, 16, 1, 16, 32, 1, 2, kDstCanary,
                                    kDataSize) ||
               !M2MFVerifyCanaries(dst_buf, kCanarySize, 0x55) ||
               !M2MFVerifyCanaries(dst + kDataSize, kCanarySize, 0xAA)) {
      pass = false;
    } else {
      LogMsg(
          "  [PASS] stride_in=1, stride_out=2 (0x0201) single-line "
          "verified.\n");
    }
  }

  // 2. stride_in = 1, stride_out = 2 (0x0201): Multi-line pitched dispersal
  {
    reset_buffers();
    for (uint8_t i = 0; i < 16; ++i) {
      src[i] = static_cast<uint8_t>(0x10 + i);
      src[16 + i] = static_cast<uint8_t>(0x20 + i);
    }

    void* ret = gpum_copy_strided(dst, src, 8, 2, 16, 32, 1, 2);
    if (!ret) {
      LogMsg("  [FAIL] 0x0201 multi-line transfer timed out or failed.\n");
      pass = false;
    } else if (!VerifyStridedOutput(dst, src, 8, 2, 16, 32, 1, 2, kDstCanary,
                                    kDataSize) ||
               !M2MFVerifyCanaries(dst_buf, kCanarySize, 0x55) ||
               !M2MFVerifyCanaries(dst + kDataSize, kCanarySize, 0xAA)) {
      pass = false;
    } else {
      LogMsg(
          "  [PASS] stride_in=1, stride_out=2 (0x0201) multi-line "
          "verified.\n");
    }
  }

  // 3. stride_in = 2, stride_out = 1 (0x0102): Single-line gathering
  {
    reset_buffers();
    for (uint8_t i = 0; i < 32; ++i) {
      src[i] = static_cast<uint8_t>(0x10 + i);
    }

    void* ret = gpum_copy_strided(dst, src, 16, 1, 32, 16, 2, 1);
    if (!ret) {
      LogMsg("  [FAIL] 0x0102 single-line transfer timed out or failed.\n");
      pass = false;
    } else if (!VerifyStridedOutput(dst, src, 16, 1, 32, 16, 2, 1, kDstCanary,
                                    kDataSize) ||
               !M2MFVerifyCanaries(dst_buf, kCanarySize, 0x55) ||
               !M2MFVerifyCanaries(dst + kDataSize, kCanarySize, 0xAA)) {
      pass = false;
    } else {
      LogMsg(
          "  [PASS] stride_in=2, stride_out=1 (0x0102) gathering verified.\n");
    }
  }

  // 4. Symmetric strides: (2, 2) and (4, 4)
  struct SymmetricCase {
    uint32_t stride;
    uint32_t line_len;
    uint32_t pitch;
  };
  const SymmetricCase kSymmetricCases[] = {
      {2, 16, 32},
      {4, 16, 64},
  };

  for (const auto& sc : kSymmetricCases) {
    reset_buffers();
    for (uint8_t i = 0; i < 64; ++i) {
      src[i] = static_cast<uint8_t>(0x30 + i);
    }

    void* ret = gpum_copy_strided(dst, src, sc.line_len, 1, sc.pitch, sc.pitch,
                                  sc.stride, sc.stride);
    if (!ret) {
      LogMsg(
          "  [FAIL] Symmetric stride (%u, %u) transfer timed out or failed.\n",
          sc.stride, sc.stride);
      pass = false;
    } else if (!VerifyStridedOutput(dst, src, sc.line_len, 1, sc.pitch,
                                    sc.pitch, sc.stride, sc.stride, kDstCanary,
                                    kDataSize) ||
               !M2MFVerifyCanaries(dst_buf, kCanarySize, 0x55) ||
               !M2MFVerifyCanaries(dst + kDataSize, kCanarySize, 0xAA)) {
      pass = false;
    } else {
      LogMsg("  [PASS] Symmetric stride (%u, %u) verified.\n", sc.stride,
             sc.stride);
    }
  }

  // 5. Asymmetric strides
  struct AsymCase {
    uint32_t stride_in;
    uint32_t stride_out;
    uint32_t line_len;
    uint32_t pitch_in;
    uint32_t pitch_out;
    const char* desc;
  };

  const AsymCase kAsymCases[] = {
      {1, 4, 16, 16, 64, "stride_in=1, stride_out=4 (0x0401)"},
      {4, 1, 16, 64, 16, "stride_in=4, stride_out=1 (0x0104)"},
      {2, 4, 16, 32, 64, "stride_in=2, stride_out=4 (0x0402)"},
      {4, 2, 16, 64, 32, "stride_in=4, stride_out=2 (0x0204)"},
  };

  for (const auto& ac : kAsymCases) {
    reset_buffers();
    for (uint8_t i = 0; i < 64; ++i) {
      src[i] = static_cast<uint8_t>(0x40 + i);
    }

    void* ret = gpum_copy_strided(dst, src, ac.line_len, 1, ac.pitch_in,
                                  ac.pitch_out, ac.stride_in, ac.stride_out);
    if (!ret) {
      LogMsg("  [FAIL] Asymmetric stride %s timed out or failed.\n", ac.desc);
      pass = false;
    } else if (!VerifyStridedOutput(dst, src, ac.line_len, 1, ac.pitch_in,
                                    ac.pitch_out, ac.stride_in, ac.stride_out,
                                    kDstCanary, kDataSize) ||
               !M2MFVerifyCanaries(dst_buf, kCanarySize, 0x55) ||
               !M2MFVerifyCanaries(dst + kDataSize, kCanarySize, 0xAA)) {
      pass = false;
    } else {
      LogMsg("  [PASS] Asymmetric stride %s verified.\n", ac.desc);
    }
  }

  MmFreeContiguousMemory(src_buf);
  MmFreeContiguousMemory(dst_buf);
  return pass;
}
