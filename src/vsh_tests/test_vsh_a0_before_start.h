#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_A0_BEFORE_START_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_A0_BEFORE_START_H

//! Characterizes the result of a vertex shader reading `c[A0 + 0]` when A0 is
//! negative and therefore addresses a location before the start of the
//! constant file.
//!
//! Observed hardware behavior: A0 = 0 returns c[0], while every negative A0
//! (-1 through -65536 are probed) returns (0, 0, 0, 0) rather than wrapping to
//! the end of the constant file. No PGRAPH interrupt is raised.
//!
//! Validates that:
//! 1. Draws using negative A0 values complete without hanging the GPU.
//! 2. The value produced by each read is recorded and classified as a loaded
//! constant (wrap/clamp/alias), zeros, or other.
//! 3. A0 = 0 (control) still returns c[0].
//! 4. NV_PGRAPH_INTR is recorded after the draw.
struct TestVshA0BeforeStart {
  static constexpr const char* Name() { return "VSH A0 Before Constant File"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_A0_BEFORE_START_H
