#include "test_alarm_rollover_and_periodicity.h"

#include "ptimer_test_common.h"

bool TestAlarmRolloverAndPeriodicity::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: Alarm Rollover & Periodicity\n");

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;

  VIDEOREG(NV_PTIMER_NUMERATOR) = kTestNumerator;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = kTestDenominator;

  static constexpr uint32_t kStartHigh = 2;
  // TODO: Investigate making this higher. xemu running in debug mode misses the
  //       window if this is set to 0xFFFFF000.
  static constexpr uint32_t kStartLow = 0xFE000000;
  static constexpr uint32_t kAlarmTarget = 0x00002000;

  VIDEOREG(NV_PTIMER_TIME_1) = kStartHigh;
  VIDEOREG(NV_PTIMER_TIME_0) = kStartLow;
  VIDEOREG(NV_PTIMER_ALARM_0) = kAlarmTarget;

  g_alarm_capture_count = 0;
  ptimer_alarm_count = 0;
  ptimer_alarm_fired_callback = CapturingAlarmCallback;

  LogMsg("Configured: TIME_1=0x%08X, TIME_0=0x%08X, ALARM_0=0x%08X\n",
         kStartHigh, kStartLow, kAlarmTarget);
  LogMsg("Waiting 5.0s across full 2^32 rollover (4.295s epoch)...\n");

  LARGE_INTEGER qpc_start, qpc_now, qpc_freq;
  QueryPerformanceFrequency(&qpc_freq);
  QueryPerformanceCounter(&qpc_start);

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;

  while (g_alarm_capture_count < 5) {
    QueryPerformanceCounter(&qpc_now);
    double elapsed_ms = (double)(qpc_now.QuadPart - qpc_start.QuadPart) *
                        1000.0 / qpc_freq.QuadPart;
    if (elapsed_ms >= 5000.0) {
      break;
    }
  }

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  ptimer_alarm_fired_callback = nullptr;

  size_t count = g_alarm_capture_count;
  LogMsg("Captured %u alarm firing(s) across 5.0s window:\n", count);

  for (size_t i = 0; i < count; ++i) {
    const auto& cap = g_alarm_captures[i];
    double ms_from_start = (double)(cap.qpc.QuadPart - qpc_start.QuadPart) *
                           1000.0 / qpc_freq.QuadPart;
    LogMsg(
        "  Firing #%u: TIME_1=0x%08X TIME_0=0x%08X ALARM=0x%08X (t=+%.3f ms)\n",
        cap.alarm_count, cap.time_1, cap.time_0, cap.alarm_reg, ms_from_start);
  }

  if (count != 2) {
    LogMsg("  [FAIL] Expected exactly 2 firings across 5.0s (got %u)!\n",
           count);
    return false;
  }

  LogMsg(
      "  [PASS] Hardware verified: Alarm periodically fires on each 2^32 "
      "rollover (~4.295s).\n");
  return true;
}
