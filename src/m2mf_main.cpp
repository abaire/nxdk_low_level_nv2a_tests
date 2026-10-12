#ifndef XBOX
#error Must be built with nxdk
#endif

#include "m2mf_tests/m2mf_test_common.h"
#include "m2mf_tests/test_m2mf_alignment_odd_sizes.h"
#include "m2mf_tests/test_m2mf_alignment_phases.h"
#include "m2mf_tests/test_m2mf_batching_causal_chain.h"
#include "m2mf_tests/test_m2mf_batching_independent.h"
#include "m2mf_tests/test_m2mf_format.h"
#include "m2mf_tests/test_m2mf_framebuffer.h"
#include "m2mf_tests/test_m2mf_large_pitch.h"
#include "m2mf_tests/test_m2mf_linear_copy.h"
#include "m2mf_tests/test_m2mf_max_line_count.h"
#include "m2mf_tests/test_m2mf_min_transfer.h"
#include "m2mf_tests/test_m2mf_notifier_record.h"
#include "m2mf_tests/test_m2mf_notifier_switching.h"
#include "m2mf_tests/test_m2mf_pitched_asymmetric.h"
#include "m2mf_tests/test_m2mf_pitched_packing.h"
#include "m2mf_tests/test_m2mf_pitched_symmetric.h"
#include "m2mf_tests/test_m2mf_sysmem_to_vram.h"
#include "m2mf_tests/test_m2mf_vram_to_sysmem.h"
#include "m2mf_tests/test_m2mf_vram_to_vram.h"
#include "m2mf_tests/test_m2mf_zero_size.h"
#include "test_runner.h"
#include "test_suite.h"

static const char* kDefaultLogPath =
    R"(e:\devkit\nxdk_low_level_nv2a_tests\m2mf_log.txt)";

static constexpr TestCase kTests[] = {
    TestCase::From<TestM2MFAlignmentOddSizes>(),
    TestCase::From<TestM2MFAlignmentPhases>(),
    TestCase::From<TestM2MFBatchingCausalChain>(),
    TestCase::From<TestM2MFBatchingIndependent>(),
    TestCase::From<TestM2MFFormat>(),
    TestCase::From<TestM2MFFramebuffer>(),
    TestCase::From<TestM2MFLargePitch>(),
    TestCase::From<TestM2MFLinearCopy>(),
    TestCase::From<TestM2MFMaxLineCount>(),
    TestCase::From<TestM2MFMinTransfer>(),
    TestCase::From<TestM2MFNotifierRecord>(),
    TestCase::From<TestM2MFNotifierSwitching>(),
    TestCase::From<TestM2MFPitchedAsymmetric>(),
    TestCase::From<TestM2MFPitchedPacking>(),
    TestCase::From<TestM2MFPitchedSymmetric>(),
    TestCase::From<TestM2MFSysmemToVram>(),
    TestCase::From<TestM2MFVramToSysmem>(),
    TestCase::From<TestM2MFVramToVram>(),
    TestCase::From<TestM2MFZeroSize>(),
};

int main() {
  TestRunner runner("M2MF", kDefaultLogPath, kTests, std::size(kTests));
  int result = runner.Run();
  M2MFTeardown();
  return result;
}
