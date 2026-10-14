#ifndef XBOX
#error Must be built with nxdk
#endif

#include "ptimer_tests/ptimer_test_common.h"
#include "ptimer_tests/test_alarm_adjust_preserves_pending_flag.h"
#include "ptimer_tests/test_alarm_flag_is_set_while_masked.h"
#include "ptimer_tests/test_alarm_flag_is_set_while_masked_reset_time.h"
#include "ptimer_tests/test_alarm_rearm.h"
#include "ptimer_tests/test_alarm_rollover_and_periodicity.h"
#include "ptimer_tests/test_alarm_set_in_the_past.h"
#include "ptimer_tests/test_alarm_target_zero.h"
#include "ptimer_tests/test_alarm_unaligned_target.h"
#include "ptimer_tests/test_alarm_unprogrammed.h"
#include "ptimer_tests/test_clock_scaling_ratios.h"
#include "ptimer_tests/test_interrupt_acknowledge.h"
#include "ptimer_tests/test_interrupt_masking.h"
#include "ptimer_tests/test_register_bitmasks.h"
#include "ptimer_tests/test_time_register_read_write.h"
#include "ptimer_tests/test_time_register_rollover.h"
#include "ptimer_tests/test_zero_divisor_clock.h"
#include "test_runner.h"
#include "test_suite.h"

static const char* kDefaultLogPath =
    R"(e:\devkit\nxdk_low_level_nv2a_tests\log.txt)";

static constexpr TestCase kTests[] = {
    TestCase::From<TestAlarmUnprogrammed>(),
    TestCase::From<TestAlarmTargetZero>(),
    TestCase::From<TestAlarmRearm>(),
    TestCase::From<TestAlarmAdjustPreservesPendingFlag>(),
    TestCase::From<TestAlarmRolloverAndPeriodicity>(),
    TestCase::From<TestAlarmSetInThePast>(),
    TestCase::From<TestAlarmUnalignedTarget>(),
    TestCase::From<TestRegisterBitmasks>(),
    TestCase::From<TestInterruptMasking>(),
    TestCase::From<TestAlarmFlagIsSetWhileMasked>(),
    TestCase::From<TestAlarmFlagIsSetWhileMaskedResetTime>(),
    TestCase::From<TestInterruptAcknowledge>(),
    TestCase::From<TestTimeRegisterReadWrite>(),
    TestCase::From<TestTimeRegisterRollover>(),
    TestCase::From<TestClockScalingRatios>(),
    TestCase::From<TestZeroDivisorClock>(),
};

int main() {
  TestRunner runner("NV2A PTIMER", kDefaultLogPath, kTests, std::size(kTests),
                    []() {
                      RunPreInitBootAlarmTest();

                      g_clock_state.Init();

                      VIDEOREG(NV_PTIMER_NUMERATOR) = kTestNumerator;
                      VIDEOREG(NV_PTIMER_DENOMINATOR) = kTestDenominator;
                    });
  return runner.Run();
}
