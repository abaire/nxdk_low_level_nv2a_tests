#include "test_alarm_target_zero.h"

#include "ptimer_test_common.h"

bool TestAlarmTargetZero::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: Alarm Target Zero\n");

  bool pass = true;

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;

  VIDEOREG(NV_PTIMER_NUMERATOR) = kTestNumerator;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = kTestDenominator;

  g_alarm_capture_count = 0;
  ptimer_alarm_count = 0;
  ptimer_alarm_fired_callback = CapturingAlarmCallback;

  static constexpr uint32_t kStartLow = 0xFF000000;

  VIDEOREG(NV_PTIMER_ALARM_0) = 0;
  VIDEOREG(NV_PTIMER_TIME_0) = kStartLow;
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;

  LARGE_INTEGER qpc_start, qpc_now, qpc_freq;
  QueryPerformanceFrequency(&qpc_freq);
  QueryPerformanceCounter(&qpc_start);

  while (g_alarm_capture_count == 0) {
    uint32_t t = VIDEOREG(NV_PTIMER_TIME_0);
    if (t < kStartLow && t > 0x01000000) {
      break;
    }

    QueryPerformanceCounter(&qpc_now);
    double elapsed_ms = (double)(qpc_now.QuadPart - qpc_start.QuadPart) *
                        1000.0 / qpc_freq.QuadPart;
    if (elapsed_ms >= 500.0) {
      break;
    }
  }

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  ptimer_alarm_fired_callback = nullptr;

  uint32_t intr_final = VIDEOREG(NV_PTIMER_INTR_0);
  uint32_t t_final = VIDEOREG(NV_PTIMER_TIME_0);

  LogMsg("  Final state: TIME_0=0x%08X, INTR_0=0x%08X, alarm firings=%u\n",
         t_final, intr_final, g_alarm_capture_count);

  if (g_alarm_capture_count > 0) {
    const auto& cap = g_alarm_captures[0];
    int32_t jitter = static_cast<int32_t>(cap.time_0);
    LogMsg(
        "  [PASS] Alarm fired for ALARM_0 = 0 at TIME_0=0x%08X (jitter: %+d "
        "ticks)\n",
        cap.time_0, jitter);
  } else {
    LogMsg(
        "  [FAIL] Alarm did not fire when TIME_0 wrapped past ALARM_0 = 0!\n");
    if ((intr_final & 1) != 0) {
      LogMsg(
          "         (INTR_0 bit 0 was set, but CPU interrupt did not fire)\n");
    }
    pass = false;
  }

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;

  return pass;
}
