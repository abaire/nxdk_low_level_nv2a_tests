#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_A0_OFFSET_BEYOND_END_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_A0_OFFSET_BEYOND_END_H

//! Characterizes the result of reading `c[A0 + 191]` when the sum of A0 and
//! the instruction's static offset lands beyond the end of the constant file.
//!
//! Complements TestVshA0BeyondEnd by showing whether the static offset and A0
//! are combined before or after any range handling.
//!
//! Observed hardware behavior: the static offset and A0 are summed before the
//! range check. Sums within [0, 191] return the matching constant, including
//! negative A0 values that the offset brings back into range (e.g. A0 = -191
//! reads c[0]). Sums of 192 or more return (0, 0, 0, 0). No PGRAPH interrupt
//! is raised.
//!
//! Validates that:
//! 1. Reads where A0 + 191 is within [0, 191] return the matching constant.
//! 2. Draws where A0 + 191 is at or beyond 192 complete without hanging the
//! GPU.
//! 3. The value produced by each out-of-range read is recorded and classified
//! as a loaded constant (wrap/clamp/alias), zeros, or other.
//! 4. NV_PGRAPH_INTR is recorded after the draw.
struct TestVshA0OffsetBeyondEnd {
  static constexpr const char* Name() {
    return "VSH A0 + Static Offset Beyond Constant File";
  }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_A0_OFFSET_BEYOND_END_H
