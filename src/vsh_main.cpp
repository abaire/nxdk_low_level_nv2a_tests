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
#include "vsh_tests/test_vsh_rounding_add.h"
#include "vsh_tests/test_vsh_rounding_dot.h"
#include "vsh_tests/test_vsh_rounding_dst.h"
#include "vsh_tests/test_vsh_rounding_mad.h"
#include "vsh_tests/test_vsh_rounding_mul.h"
#include "vsh_tests/test_vsh_rugby_projection.h"

static const char* kDefaultLogPath =
    R"(e:\devkit\nxdk_low_level_nv2a_tests\vsh_log.json)";

static constexpr TestCase kTests[] = {
    TestCase::From<TestVshA0BeforeStart>(),
    TestCase::From<TestVshA0BeyondEnd>(),
    TestCase::From<TestVshA0OffsetBeyondEnd>(),
    TestCase::From<TestVshLogNegativeExponent>(),
    TestCase::From<TestVshLogDenormal>(),
    TestCase::From<TestVshRoundingMul>(),
    TestCase::From<TestVshRoundingAdd>(),
    TestCase::From<TestVshRoundingDst>(),
    TestCase::From<TestVshRoundingMad>(),
    TestCase::From<TestVshRoundingDot>(),
    TestCase::From<TestVshRoundingDotAccumulation>(),
    TestCase::From<TestVshRugbyProjection>(),
};

int main() {
  TestRunner runner("VSH", kDefaultLogPath, kTests, std::size(kTests));
  return runner.Run();
}
