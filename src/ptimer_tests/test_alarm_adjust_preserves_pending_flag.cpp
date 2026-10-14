#include "test_alarm_adjust_preserves_pending_flag.h"

#include "ptimer_test_common.h"

bool TestAlarmAdjustPreservesPendingFlag::Test() {
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

  TEST_INFO("Arming initial alarm: t0=0x%08X, ALARM_0=0x%08X, t_armed=0x%08X",
            t0, target_alarm, t_armed);

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

  if (!latched) {
    uint32_t intr_final = VIDEOREG(NV_PTIMER_INTR_0);
    TEST_FAIL_EX(TestFields("intr_0", Hex32(intr_final)),
                 "Initial alarm did not latch! (INTR_0=0x%08X)", intr_final);
    VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
    VIDEOREG(NV_PTIMER_INTR_0) = 1;
    ptimer_alarm_fired_callback = nullptr;
    return false;
  }

  TEST_INFO("Initial alarm latched at TIME_0=0x%08X", t_latched);

  if (g_alarm_capture_count != 0) {
    TEST_FAIL("CPU interrupt fired while INTR_EN_0 was 0!");
    pass = false;
  }

  uint32_t intr_before_adjust = VIDEOREG(NV_PTIMER_INTR_0);
  uint32_t far_future_alarm = (t_latched + 0x20000000) & ALARM_MASK;
  VIDEOREG(NV_PTIMER_ALARM_0) = far_future_alarm;
  uint32_t intr_after_adjust = VIDEOREG(NV_PTIMER_INTR_0);

  TEST_INFO("INTR_0 before adjust: 0x%08X", intr_before_adjust);
  TEST_INFO("Reprogrammed ALARM_0: 0x%08X (far future)", far_future_alarm);
  TEST_INFO("INTR_0 after adjust : 0x%08X", intr_after_adjust);

  if ((intr_after_adjust & 1) == 0) {
    TEST_FAIL("Adjusting ALARM_0 cleared pending INTR_0 bit 0!");
    pass = false;
  } else {
    TEST_PASS("Pending INTR_0 bit 0 remained asserted after ALARM_0 write.");

    VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;
    Sleep(5);
    VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;

    TEST_INFO("CPU interrupt firings after unmasking: %u",
              g_alarm_capture_count);
    if (g_alarm_capture_count > 0) {
      TEST_PASS(
          "CPU interrupt fired upon unmasking with preserved pending flag.");
    } else {
      TEST_FAIL("CPU interrupt did not fire upon unmasking!");
      pass = false;
    }
  }

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;
  ptimer_alarm_fired_callback = nullptr;

  return pass;
}
