#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_PAIRED_ILU_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_PAIRED_ILU_H

//! Validates which temporary register receives the ILU half of a paired
//! MAC + ILU instruction.
//!
//! Ported from nxdk_vsh_tests (src/tests/paired_ilu_tests.cpp). Expected
//! values are the bit patterns observed on hardware.
//!
//! Observed hardware behavior: the ILU result of a paired instruction is always
//! written to R1, even when the instruction encodes a different temporary
//! (here R10). The encoded register is left untouched. RSQ(25) is 0x3E4CCCCC,
//! one ULP below the nearest float to 0.2 (0x3E4CCCCD).
//!
//! Validates that:
//! 1. `dp4 ... + rsq r1.x, 25` writes RSQ(25) = 0x3E4CCCCC to R1.x.
//! 2. R1 and R10 can be reset to the input value (25, 1.123, 2.123, 3.123).
//! 3. `dp4 ... + rsq r10.x, 25` writes 0x3E4CCCCC to R1.x and leaves R1.yzw
//! intact.
//! 4. R10 is unchanged by the paired instruction targeting it.
struct TestVshPairedIlu {
  static constexpr const char* Name() { return "VSH Paired ILU R1 Write"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_PAIRED_ILU_H
