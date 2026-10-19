#ifndef XBOX
#error Must be built with nxdk
#endif

#include "test_runner.h"
#include "test_suite.h"
#include "vsh_tests/test_vsh_a0_before_start.h"
#include "vsh_tests/test_vsh_a0_beyond_end.h"
#include "vsh_tests/test_vsh_a0_offset_beyond_end.h"
#include "vsh_tests/test_vsh_americas_army.h"
#include "vsh_tests/test_vsh_ilu_rcp.h"
#include "vsh_tests/test_vsh_inline_exceptional_float.h"
#include "vsh_tests/test_vsh_log_denormal.h"
#include "vsh_tests/test_vsh_log_negative_exponent.h"
#include "vsh_tests/test_vsh_mac_add.h"
#include "vsh_tests/test_vsh_mac_mov.h"
#include "vsh_tests/test_vsh_paired_ilu.h"
#include "vsh_tests/test_vsh_spy_vs_spy_menu.h"
#include "vsh_tests/test_vsh_vertex_array_format.h"

static const char* kDefaultLogPath =
    R"(e:\devkit\nxdk_low_level_nv2a_tests\vsh_log.json)";

static constexpr TestCase kTests[] = {
    TestCase::From<TestVshA0BeforeStart>(),
    TestCase::From<TestVshA0BeyondEnd>(),
    TestCase::From<TestVshA0OffsetBeyondEnd>(),
    TestCase::From<TestVshLogNegativeExponent>(),
    TestCase::From<TestVshLogDenormal>(),
    TestCase::From<TestVshMacMov>(),
    TestCase::From<TestVshMacAdd>(),
    TestCase::From<TestVshIluRcp>(),
    TestCase::From<TestVshPairedIlu>(),
    TestCase::From<TestVshAmericasArmy>(),
    TestCase::From<TestVshSpyVsSpyMenu>(),
    TestCase::From<TestVshInlineExceptionalFloat>(),
    TestCase::From<TestVshVertexArrayFormatF>(),
    TestCase::From<TestVshVertexArrayFormatS1>(),
    TestCase::From<TestVshVertexArrayFormatS32K>(),
};

int main() {
  TestRunner runner("VSH", kDefaultLogPath, kTests, std::size(kTests));
  return runner.Run();
}
