#include "test_interrupt_masking.h"

#include "ptimer_test_common.h"

bool TestInterruptMasking::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: Interrupt Masking & Pending Latching\n");

  bool pass = true;

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;

  VIDEOREG(NV_PTIMER_NUMERATOR) = kTestNumerator;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = kTestDenominator;

  VIDEOREG(NV_PTIMER_TIME_1) = 1;
  VIDEOREG(NV_PTIMER_TIME_0) = 0x00001000;
  VIDEOREG(NV_PTIMER_ALARM_0) = 0x00020000;

  g_alarm_capture_count = 0;
  ptimer_alarm_fired_callback = CapturingAlarmCallback;

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;

  LogMsg(
      "Disarmed INTR_EN_0 in-flight; waiting for TIME_0 to cross ALARM_0...\n");
  while (VIDEOREG(NV_PTIMER_TIME_0) < 0x00030000) {
  }

  LogMsg("  Firings while disabled: %u\n", g_alarm_capture_count);
  if (g_alarm_capture_count != 0) {
    LogMsg("  [FAIL] CPU interrupt fired while INTR_EN_0 was disabled!\n");
    pass = false;
  } else {
    LogMsg("  [PASS] No CPU interrupt fired while disabled.\n");
  }

  uint32_t pending_masked = VIDEOREG(NV_PTIMER_INTR_0);
  LogMsg("  INTR_0 after crossing threshold: 0x%08X (bit 0 expected = 1)\n",
         pending_masked);
  if ((pending_masked & 1) == 0) {
    LogMsg("  [FAIL] Pending bit did NOT latch when masked.\n");
    pass = false;
  } else {
    LogMsg(
        "  [PASS] Pending bit correctly latched while interrupt was masked.\n");
  }

  if (pending_masked & 1) {
    LogMsg("Re-enabling INTR_EN_0 = 1 with pending bit set...\n");
    VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;
    Sleep(5);
    VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
    ptimer_alarm_fired_callback = nullptr;

    LogMsg("  Alarm firings: %u\n", g_alarm_capture_count);
    if (g_alarm_capture_count > 0) {
      LogMsg(
          "  [PASS] CPU interrupt fired upon enabling with pending bit set.\n");
    } else {
      LogMsg("  [FAIL] CPU interrupt did not fire upon unmasking!\n");
      pass = false;
    }
  }

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;

  return pass;
}
