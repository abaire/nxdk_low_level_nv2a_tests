#include "test_alarm_rearm.h"

#include "ptimer_test_common.h"

bool TestAlarmRearm::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: Explicit Alarm Re-arming & Triggering\n");

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;

  VIDEOREG(NV_PTIMER_NUMERATOR) = kTestNumerator;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = kTestDenominator;

  g_alarm_capture_count = 0;
  ptimer_alarm_count = 0;
  ptimer_alarm_fired_callback = CapturingAlarmCallback;

  uint32_t now_low = VIDEOREG(NV_PTIMER_TIME_0);
  uint32_t target_rearm = (now_low + 0x00200000) & ALARM_MASK;

  VIDEOREG(NV_PTIMER_ALARM_0) = target_rearm;
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;

  LogMsg("Testing explicit re-arm: set TIME_0=0x%08X -> ALARM_0=0x%08X\n",
         now_low, target_rearm);

  LARGE_INTEGER qpc_start, qpc_now, qpc_freq;
  QueryPerformanceFrequency(&qpc_freq);
  QueryPerformanceCounter(&qpc_start);

  while (g_alarm_capture_count == 0) {
    QueryPerformanceCounter(&qpc_now);
    double elapsed_ms = (double)(qpc_now.QuadPart - qpc_start.QuadPart) *
                        1000.0 / qpc_freq.QuadPart;
    if (elapsed_ms >= 100.0) {
      break;
    }
  }

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  ptimer_alarm_fired_callback = nullptr;

  if (g_alarm_capture_count <= 0) {
    LogMsg("  [FAIL] Explicitly reprogrammed ALARM_0 failed to fire!\n");
    return false;
  }

  LogMsg("  [PASS] Explicitly reprogrammed ALARM_0 successfully fired.\n");
  return true;
}
