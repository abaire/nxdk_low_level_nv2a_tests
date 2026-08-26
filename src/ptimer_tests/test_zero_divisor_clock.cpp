#include "test_zero_divisor_clock.h"

#include "ptimer_test_common.h"

bool TestZeroDivisorClock::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: Zero-Divisor Clock Configurations\n");

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;

  VIDEOREG(NV_PTIMER_NUMERATOR) = 0;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = 1000;

  uint32_t t0_zero_num = VIDEOREG(NV_PTIMER_TIME_0);
  uint32_t t1_zero_num = VIDEOREG(NV_PTIMER_TIME_1);
  VIDEOREG(NV_PTIMER_ALARM_0) = t0_zero_num + 0x1000;
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;
  Sleep(10);
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;

  LogMsg("NUMERATOR = 0, DENOMINATOR = 1000: TIME_0=0x%08X, TIME_1=0x%08X\n",
         t0_zero_num, t1_zero_num);

  VIDEOREG(NV_PTIMER_NUMERATOR) = 1000;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = 0;

  uint32_t t0_zero_den = VIDEOREG(NV_PTIMER_TIME_0);
  uint32_t t1_zero_den = VIDEOREG(NV_PTIMER_TIME_1);
  VIDEOREG(NV_PTIMER_ALARM_0) = t0_zero_den + 0x1000;
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;
  Sleep(10);
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;

  LogMsg("NUMERATOR = 1000, DENOMINATOR = 0: TIME_0=0x%08X, TIME_1=0x%08X\n",
         t0_zero_den, t1_zero_den);

  VIDEOREG(NV_PTIMER_NUMERATOR) = 0;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = 0;

  uint32_t t0_zero_both = VIDEOREG(NV_PTIMER_TIME_0);
  uint32_t t1_zero_both = VIDEOREG(NV_PTIMER_TIME_1);
  VIDEOREG(NV_PTIMER_ALARM_0) = t0_zero_both + 0x1000;

  LogMsg("NUMERATOR = 0, DENOMINATOR = 0: TIME_0=0x%08X, TIME_1=0x%08X\n",
         t0_zero_both, t1_zero_both);

  VIDEOREG(NV_PTIMER_NUMERATOR) = kTestNumerator;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = kTestDenominator;

  LogMsg(
      "  [PASS] Zero-divisor register configurations handled without crash.\n");
  return true;
}
