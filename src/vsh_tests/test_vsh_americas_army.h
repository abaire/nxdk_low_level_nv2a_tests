#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_AMERICAS_ARMY_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_AMERICAS_ARMY_H

//! Runs an excerpt of a vertex shader from America's Army (xemu#1008) in which
//! RCP(0) = +Inf is subsequently multiplied by a zero constant.
//!
//! Ported from nxdk_vsh_tests (src/tests/americasarmyshader.cpp). Expected
//! values are the bit patterns observed on hardware.
//!
//! Observed hardware behavior: Inf * 0 in a MAC MUL produces 0 rather than NaN,
//! so the result does not poison later computations.
//!
//! Validates that:
//! 1. `mul c[188].xyz, R1.y (= Inf), c[140].xyz (= 0)` produces +0.
//! 2. `mov c[188].w, R1.y` produces +Inf, confirming RCP(0) == +Inf.
struct TestVshAmericasArmy {
  static constexpr const char* Name() { return "VSH Americas Army Inf*0"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_AMERICAS_ARMY_H
