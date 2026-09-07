#include "test_alarm_flag_is_set_while_masked.h"

#include "ptimer_test_common.h"

bool TestAlarmFlagIsSetWhileMasked::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: Alarm Flag Is Set While Masked\n");

  bool pass = true;

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;

  VIDEOREG(NV_PTIMER_NUMERATOR) = kTestNumerator;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = kTestDenominator;

  g_alarm_capture_count = 0;
  ptimer_alarm_count = 0;
  ptimer_alarm_fired_callback = CapturingAlarmCallback;

  static constexpr uint32_t kAlarmOffset = 10000000;
  uint32_t t0 = VIDEOREG(NV_PTIMER_TIME_0);
  uint32_t target_alarm = (t0 + kAlarmOffset) & ALARM_MASK;
  VIDEOREG(NV_PTIMER_ALARM_0) = target_alarm;
  uint32_t t_armed = VIDEOREG(NV_PTIMER_TIME_0);

  LogMsg("  Arming: t0=0x%08X, ALARM_0=0x%08X, t_armed=0x%08X\n", t0,
         target_alarm, t_armed);

  LARGE_INTEGER qpc_start, qpc_now, qpc_freq;
  QueryPerformanceFrequency(&qpc_freq);
  QueryPerformanceCounter(&qpc_start);

  bool latched = false;
  uint32_t t_latched = 0;

  while (true) {
    uint32_t intr = VIDEOREG(NV_PTIMER_INTR_0);
    if ((intr & 1) != 0) {
      latched = true;
      t_latched = VIDEOREG(NV_PTIMER_TIME_0);
      break;
    }

    uint32_t now = VIDEOREG(NV_PTIMER_TIME_0);
    if ((uint32_t)(now - t_armed) > (kAlarmOffset + 5000000)) {
      break;
    }

    QueryPerformanceCounter(&qpc_now);
    double elapsed_ms = (double)(qpc_now.QuadPart - qpc_start.QuadPart) *
                        1000.0 / qpc_freq.QuadPart;
    if (elapsed_ms >= 500.0) {
      break;
    }
  }

  LogMsg("  CPU interrupt firings: %u\n", g_alarm_capture_count);
  if (g_alarm_capture_count != 0) {
    LogMsg("  [FAIL] CPU interrupt fired while INTR_EN_0 was 0!\n");
    pass = false;
  } else {
    LogMsg("  [PASS] No CPU interrupt fired while INTR_EN_0 was 0.\n");
  }

  if (!latched) {
    uint32_t intr_final = VIDEOREG(NV_PTIMER_INTR_0);
    LogMsg("  [FAIL] INTR_0 bit 0 did not latch! (INTR_0=0x%08X)\n",
           intr_final);
    pass = false;
  } else {
    int32_t jitter = (int32_t)(t_latched - target_alarm);
    LogMsg(
        "  [PASS] Polling detected alarm latched at TIME_0=0x%08X (jitter: %+d "
        "ticks)\n",
        t_latched, jitter);

    VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;
    Sleep(5);
    VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;

    LogMsg("  CPU interrupt firings after enable: %u\n", g_alarm_capture_count);
    if (g_alarm_capture_count > 0) {
      LogMsg("  [PASS] CPU interrupt fired upon unmasking with pending bit.\n");
    } else {
      LogMsg("  [FAIL] CPU interrupt failed to fire upon unmasking!\n");
      pass = false;
    }
  }

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;
  ptimer_alarm_fired_callback = nullptr;

  return pass;
}
