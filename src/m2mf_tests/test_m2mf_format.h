#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_FORMAT_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_FORMAT_H

//! Validates hardware behavior of class 0x39 NV_MEMORY_TO_MEMORY_FORMAT_FORMAT
//! (method 0x324) across various strided configurations.
//!
//! Validates that:
//! 1. Transfers with stride_in = 1, stride_out = 2 (FORMAT 0x0201) correctly
//! disperse 1-byte elements with a 2-byte output stride, leaving interleaved
//! gap bytes and line padding untouched.
//! 2. Transfers with stride_in = 2, stride_out = 1 (FORMAT 0x0102) correctly
//! gather every second byte from source into contiguous destination bytes.
//! 3. Transfers with symmetric strides (e.g. stride_in = stride_out = 2, 4)
//! copy every Nth byte from source to every Nth byte in destination, leaving
//! intermediate bytes untouched.
//! 4. Asymmetric strided transfers with arbitrary ratios (e.g. 1:4, 4:1, 2:4,
//! 4:2) match expected strided indexing for all written bytes while preserving
//! unwritten gap bytes.
//! 5. All strided transfers preserve boundary canaries outside the active
//! destination buffer.
struct TestM2MFFormat {
  static constexpr const char* Name() { return "M2MF Format Variations"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_FORMAT_H
