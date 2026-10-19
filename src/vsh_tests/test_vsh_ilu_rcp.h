#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ILU_RCP_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ILU_RCP_H

//! Validates the ILU RCP operation on ordinary and exceptional inputs.
//!
//! Ported from nxdk_vsh_tests (src/tests/ilu_rcp_tests.cpp). Expected values
//! are the bit patterns observed on hardware.
//!
//! Observed hardware behavior: reciprocals that underflow (1/+-Max, 1/+-Inf)
//! return a zero with the sign of the input, denormal inputs behave as signed
//! zeros (1/-MaxSub = -Inf, 1/MinSub = +Inf), and any NaN input returns the
//! canonical NaN 0x7FFFFFFF.
//!
//! Validates that:
//! 1. RCP(+-1) == +-1 and RCP(+-2.123) == +-0x3EF12B0D.
//! 2. RCP(+-2^-126) == +-2^126.
//! 3. RCP(+FLT_MAX) and RCP(+Inf) == +0; RCP(-FLT_MAX) and RCP(-Inf) == -0.
//! 4. RCP(0) == +Inf.
//! 5. RCP(NaN) and RCP(-NaN) == 0x7FFFFFFF.
//! 6. RCP(-MaxSubnormal) == -Inf, RCP(MinSubnormal) == +Inf and
//! RCP(-MinSubnormal) == -Inf.
struct TestVshIluRcp {
  static constexpr const char* Name() { return "VSH ILU RCP"; }
  static constexpr const char* Source() { return __FILE__; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_VSH_ILU_RCP_H
