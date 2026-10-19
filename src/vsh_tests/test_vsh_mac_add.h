#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_MAC_ADD_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_MAC_ADD_H

//! Validates the MAC ADD operation on ordinary finite values.
//!
//! Ported from nxdk_vsh_tests (src/tests/mac_add_tests.cpp). Expected values
//! are the bit patterns observed on hardware.
//!
//! Observed hardware behavior: -2.33 + -100 produces 0xC2CCA8F5, one ULP
//! closer to zero than the IEEE round-to-nearest result (0xC2CCA8F6),
//! suggesting the MAC truncates rather than rounds.
//!
//! Validates that `add` of (1, 2, -1, -2.33) and (1000.5, 2424.99, 1, -100)
//! produces:
//! 1. x == 1001.5 (0x447A6000).
//! 2. y == 0x4517AFD7 (2426.99).
//! 3. z == +0.
//! 4. w == 0xC2CCA8F5 (-102.33).
struct TestVshMacAdd {
  static constexpr const char* Name() { return "VSH MAC ADD"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_MAC_ADD_H
