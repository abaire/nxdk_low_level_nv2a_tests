#include "test_alarm_set_in_the_past.h"

#include "ptimer_test_common.h"

bool TestAlarmSetInThePast::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: Alarm Set in the Past\n");

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;

  VIDEOREG(NV_PTIMER_NUMERATOR) = kTestNumerator;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = kTestDenominator;

  VIDEOREG(NV_PTIMER_TIME_1) = 1;
  VIDEOREG(NV_PTIMER_TIME_0) = 0x20000000;
  VIDEOREG(NV_PTIMER_ALARM_0) = 0x10000000;

  uint32_t intr_before = VIDEOREG(NV_PTIMER_INTR_0);
  LogMsg(
      "Set TIME_0=0x20000000, ALARM_0=0x10000000. INTR_0 before enable: "
      "0x%08X\n",
      intr_before);

  ptimer_alarm_count = 0;
  g_alarm_capture_count = 0;
  ptimer_alarm_fired_callback = CapturingAlarmCallback;

  LARGE_INTEGER qpc_start, qpc_now, qpc_freq;
  QueryPerformanceFrequency(&qpc_freq);
  QueryPerformanceCounter(&qpc_start);

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;

  while (g_alarm_capture_count == 0) {
    QueryPerformanceCounter(&qpc_now);
    double elapsed_ms = (double)(qpc_now.QuadPart - qpc_start.QuadPart) *
                        1000.0 / qpc_freq.QuadPart;
    if (elapsed_ms >= 100.0) {
      break;
    }
  }

  uint32_t intr_after = VIDEOREG(NV_PTIMER_INTR_0);
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  ptimer_alarm_fired_callback = nullptr;

  LogMsg("Result after 100ms: Alarm firings=%u, INTR_0=0x%08X\n",
         g_alarm_capture_count, intr_after);

  if (g_alarm_capture_count == 0 && (intr_after & 1) == 0) {
    LogMsg(
        "  [PASS] Hardware verified: Past alarm does not fire immediately.\n");
    return true;
  }

  LogMsg("  [FAIL] Past alarm incorrectly fired immediately!\n");
  return false;
}
