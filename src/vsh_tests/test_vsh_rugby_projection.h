#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_RUGBY_PROJECTION_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_RUGBY_PROJECTION_H

//! Runs the final-draw projection and screen-space epilogue from Rugby 2005
//! (xemu-project/xemu#2388), whose clip-space z depends on DP4 truncating a
//! product.
//!
//! The projection row c[2] = (0, 0, 0xBF80309E, 0xC2203CC6) is applied to
//! v0 = (100, 50, -40, 1). The exact product 0xBF80309E * -40 lies exactly
//! halfway between 0x42203CC5 and 0x42203CC6 (its round-to-nearest value, which
//! the w term cancels), so round-to-nearest products give z = 0 while
//! truncated products give z = -2^-18.
//!
//! Validates that:
//! 1. R12 after the DP4 projection is (100, 50, -2^-18 (0xB6800000), 40).
//! 2. R12 after `mul oPos.xyz, R12, c[58]` with c[58] = (320, -240, 16777215)
//! is (32000, -12000, 0xC27FFFFF, 40).
//! 3. R1.x = RCC(40) is 0x3CCCCCCC, one ulp below the nearest float to 1/40.
//! 4. R12 after the final `mad oPos.xyz, R12, R1.x, c[59]` with
//! c[59] = (319.53125, 239.53125, 0) is
//! (0x448BF0FF, 0xC271DFF8, 0xBFCCCCCB, 40). Round-to-nearest arithmetic would
//! have given z = 0 instead of -1.6.
struct TestVshRugbyProjection {
  static constexpr const char* Name() { return "VSH Rugby 2005 Projection"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_RUGBY_PROJECTION_H
