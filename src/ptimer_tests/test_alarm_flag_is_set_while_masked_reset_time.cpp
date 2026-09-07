#include "test_alarm_flag_is_set_while_masked_reset_time.h"

#include "ptimer_test_common.h"

bool TestAlarmFlagIsSetWhileMaskedResetTime::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: Alarm Flag Is Set While Masked (Reset Time)\n");

  bool pass = true;

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;

  VIDEOREG(NV_PTIMER_NUMERATOR) = kTestNumerator;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = kTestDenominator;

  g_alarm_capture_count = 0;
  ptimer_alarm_count = 0;
  ptimer_alarm_fired_callback = CapturingAlarmCallback;

  VIDEOREG(NV_PTIMER_TIME_0) = 0x00001000;
  VIDEOREG(NV_PTIMER_ALARM_0) = 0x00100000;

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
    if (now >= 0x00200000) {
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
    int32_t jitter = (int32_t)(t_latched - 0x00100000);
    LogMsg(
        "  [PASS] Polling detected alarm latched at TIME_0=0x%08X (jitter: %+d "
        "ticks)\n",
        t_latched, jitter);
  }

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;
  ptimer_alarm_fired_callback = nullptr;

  return pass;
}
