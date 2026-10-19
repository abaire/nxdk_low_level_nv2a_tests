#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_A0_BEYOND_END_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_A0_BEYOND_END_H

//! Characterizes the result of a vertex shader reading `c[A0 + 0]` when A0
//! addresses a location beyond the end of the 192 entry constant file.
//!
//! The shader constant file is loaded with a distinct, identifiable value in
//! every register. Each A0 value is probed in its own quad and the resulting
//! diffuse color is read back from the framebuffer.
//!
//! Observed hardware behavior: reads at A0 in [0, 191] return c[A0]; every read
//! at A0 >= 192 (192 through 1000000 are probed) returns (0, 0, 0, 0) rather
//! than wrapping, clamping or aliasing a loaded constant. No PGRAPH interrupt
//! is raised.
//!
//! Validates that:
//! 1. Reads at A0 values within [0, 191] return the matching constant (control
//! for the harness).
//! 2. Draws using A0 values at and beyond 192 complete without hanging the
//! GPU.
//! 3. The value produced by each out-of-range read is recorded and classified
//! as a loaded constant (wrap/clamp/alias), zeros, or other.
//! 4. NV_PGRAPH_INTR is recorded after the draw to show whether an
//! out-of-range read raises a PGRAPH exception.
struct TestVshA0BeyondEnd {
  static constexpr const char* Name() { return "VSH A0 Beyond Constant File"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_A0_BEYOND_END_H
