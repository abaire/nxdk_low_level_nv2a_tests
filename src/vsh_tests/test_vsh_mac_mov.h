#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_MAC_MOV_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_MAC_MOV_H

//! Validates the MAC MOV operation, including a masked write, by copying a
//! constant register into the output constants.
//!
//! Ported from nxdk_vsh_tests (src/tests/mac_mov_tests.cpp), whose hardware
//! golden shows c[188] = (1, 2, -3, -4.12345).
//!
//! Validates that:
//! 1. `mov c[188], c[96]` copies all four components bit-exactly.
//! 2. `mov c[189].xy, c[96]` writes only x and y, leaving z and w unchanged.
struct TestVshMacMov {
  static constexpr const char* Name() { return "VSH MAC MOV"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_MAC_MOV_H
