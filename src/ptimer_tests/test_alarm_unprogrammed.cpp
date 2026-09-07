#include "test_alarm_unprogrammed.h"

#include "ptimer_test_common.h"

bool TestAlarmUnprogrammed::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: Alarm Unprogrammed at Boot\n");

  if (!g_boot_ptimer_state.pre_init_tested) {
    LogMsg("  [FAIL] Pre-init boot PTIMER test did not execute!\n");
    return false;
  }

  LogMsg("  Boot state captured at main() entry (prior to pb_init):\n");
  LogMsg("    NV_PTIMER_ALARM_0   : 0x%08X\n", g_boot_ptimer_state.alarm_0);
  LogMsg("    NV_PTIMER_INTR_EN_0 : 0x%08X\n", g_boot_ptimer_state.intr_en_0);
  LogMsg("    NV_PTIMER_INTR_0    : 0x%08X%s\n", g_boot_ptimer_state.intr_0,
         (g_boot_ptimer_state.intr_0 & 1) ? " (ALARM PENDING)" : "");
  LogMsg("    NV_PTIMER_TIME_1    : 0x%08X\n", g_boot_ptimer_state.time_1);
  LogMsg("    NV_PTIMER_TIME_0    : 0x%08X\n", g_boot_ptimer_state.time_0);
  LogMsg("    NV_PTIMER_NUMERATOR : 0x%08X\n", g_boot_ptimer_state.numerator);
  LogMsg("    NV_PTIMER_DENOM     : 0x%08X\n", g_boot_ptimer_state.denominator);

  bool pass = true;

  if (g_boot_ptimer_state.alarm_latched) {
    LogMsg(
        "  [PASS] Pre-pb_init: Untouched boot alarm (0x%08X) latched INTR_0 at "
        "TIME_0=0x%08X (jitter: %+d ticks, %.1f ms)\n",
        g_boot_ptimer_state.alarm_0, g_boot_ptimer_state.time_0_latched,
        g_boot_ptimer_state.jitter, g_boot_ptimer_state.elapsed_ms);
  } else {
    LogMsg(
        "  [FAIL] Pre-pb_init: Untouched boot alarm (0x%08X) failed to latch "
        "INTR_0 bit 0!\n",
        g_boot_ptimer_state.alarm_0);
    pass = false;
  }

  LogMsg("  Testing post-pb_init CPU interrupt delivery with boot ALARM_0:\n");

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;

  VIDEOREG(NV_PTIMER_NUMERATOR) = kTestNumerator;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = kTestDenominator;

  g_alarm_capture_count = 0;
  ptimer_alarm_count = 0;
  ptimer_alarm_fired_callback = CapturingAlarmCallback;

  uint32_t target_alarm = g_boot_ptimer_state.alarm_0;
  static constexpr uint32_t kOffset = 0x01000000;
  uint32_t start_time = (target_alarm - kOffset) & ALARM_MASK;

  VIDEOREG(NV_PTIMER_ALARM_0) = target_alarm;
  VIDEOREG(NV_PTIMER_TIME_0) = start_time;
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;

  LARGE_INTEGER qpc_start, qpc_now, qpc_freq;
  QueryPerformanceFrequency(&qpc_freq);
  QueryPerformanceCounter(&qpc_start);

  while (g_alarm_capture_count == 0) {
    uint32_t delta = VIDEOREG(NV_PTIMER_TIME_0) - start_time;
    if (delta > (kOffset + 0x01000000)) {
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
    int32_t jitter = static_cast<int32_t>(cap.time_0 - target_alarm);
    LogMsg(
        "  [PASS] Unprogrammed boot alarm (0x%08X) fired at TIME_0=0x%08X "
        "(jitter: %+d ticks)\n",
        target_alarm, cap.time_0, jitter);
  } else {
    LogMsg(
        "  [FAIL] Unprogrammed boot alarm (0x%08X) did not fire when TIME_0 "
        "crossed target!\n",
        target_alarm);
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
