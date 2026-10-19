#ifndef XBOX
#error Must be built with nxdk
#endif

#include "test_runner.h"
#include "test_suite.h"
#include "vsh_tests/test_vsh_a0_before_start.h"
#include "vsh_tests/test_vsh_a0_beyond_end.h"
#include "vsh_tests/test_vsh_a0_offset_beyond_end.h"
#include "vsh_tests/test_vsh_log_denormal.h"
#include "vsh_tests/test_vsh_log_negative_exponent.h"

static const char* kDefaultLogPath =
    R"(e:\devkit\nxdk_low_level_nv2a_tests\vsh_log.json)";

static constexpr TestCase kTests[] = {
    TestCase::From<TestVshA0BeforeStart>(),
    TestCase::From<TestVshA0BeyondEnd>(),
    TestCase::From<TestVshA0OffsetBeyondEnd>(),
    TestCase::From<TestVshLogNegativeExponent>(),
    TestCase::From<TestVshLogDenormal>(),
};

int main() {
  TestRunner runner("VSH", kDefaultLogPath, kTests, std::size(kTests));
  return runner.Run();
}
