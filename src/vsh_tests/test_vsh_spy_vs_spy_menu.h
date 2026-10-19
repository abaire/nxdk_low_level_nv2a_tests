#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_SPY_VS_SPY_MENU_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_SPY_VS_SPY_MENU_H

//! Runs an excerpt of the Spy vs Spy menu vertex shader, which reads back the
//! partially written oPos through R12 and feeds a vertex data array of S32K
//! values through swizzled MUL/DP4 operations.
//!
//! Ported from nxdk_vsh_tests (src/tests/spyvsspymenu.cpp). Expected values
//! are the bit patterns observed on hardware. They are within one ULP of a CPU
//! evaluation that assumes R12 reads return the current value of oPos.
//!
//! Validates that:
//! 1. `mul r0.yzw, v1.wxyz, c[120].zx` with v1 = S32K (89, 5156, -151, 417)
//! produces R0 = (1, 89, 5156, -151) once R0.x is set from c[125].z.
//! 2. The DP4s against the identity rows c[96..98] produce
//! R11.xyz = (89, 5156, -151).
//! 3. `rcc r1.x, r12.w` returns 0xB72E5201 (-1.03903e-05), RCC of the oPos.w
//! just written by `dp4 oPos.w, r0.yzwx, c[99]`.
//! 4. R12 reflects each preceding oPos write, so after
//! `mul oPos.xyz, r12.xyz, c[58]` and `mad oPos.xyz, r12.xyz, r1.x, c[59]` it
//! holds (0x439FDE1F, 0x437CE37C, 0x46CDA4BB, 0xC7BBF9CC), i.e.
//! ((R11.xyz * c[58].xyz) * R1.x + c[59].xyz, oPos.w).
struct TestVshSpyVsSpyMenu {
  static constexpr const char* Name() { return "VSH Spy vs Spy Menu R12"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_SPY_VS_SPY_MENU_H
