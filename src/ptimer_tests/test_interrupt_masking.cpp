#include "test_interrupt_masking.h"

#include "ptimer_test_common.h"

bool TestInterruptMasking::Test() {
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

  TEST_INFO(
      "Disarmed INTR_EN_0 in-flight; waiting for TIME_0 to cross ALARM_0...");
  while (VIDEOREG(NV_PTIMER_TIME_0) < 0x00030000) {
  }

  TEST_INFO("Firings while disabled: %u", g_alarm_capture_count);
  if (g_alarm_capture_count != 0) {
    TEST_FAIL("CPU interrupt fired while INTR_EN_0 was disabled!");
    pass = false;
  } else {
    TEST_PASS("No CPU interrupt fired while disabled.");
  }

  uint32_t pending_masked = VIDEOREG(NV_PTIMER_INTR_0);
  TEST_INFO("INTR_0 after crossing threshold: 0x%08X (bit 0 expected = 1)",
            pending_masked);
  if ((pending_masked & 1) == 0) {
    TEST_FAIL("Pending bit did NOT latch when masked.");
    pass = false;
  } else {
    TEST_PASS("Pending bit correctly latched while interrupt was masked.");
  }

  if (pending_masked & 1) {
    TEST_INFO("Re-enabling INTR_EN_0 = 1 with pending bit set...");
    VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;
    Sleep(5);
    VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
    ptimer_alarm_fired_callback = nullptr;

    TEST_INFO("Alarm firings: %u", g_alarm_capture_count);
    if (g_alarm_capture_count > 0) {
      TEST_PASS("CPU interrupt fired upon enabling with pending bit set.");
    } else {
      TEST_FAIL("CPU interrupt did not fire upon unmasking!");
      pass = false;
    }
  }

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;

  return pass;
}
