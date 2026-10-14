#include "test_alarm_unaligned_target.h"

#include "ptimer_test_common.h"

bool TestAlarmUnalignedTarget::Test() {
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;
  VIDEOREG(NV_PTIMER_TIME_0) = 0x00002000;
  VIDEOREG(NV_PTIMER_ALARM_0) = 0x0000201F;

  g_alarm_capture_count = 0;
  ptimer_alarm_fired_callback = CapturingAlarmCallback;
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;
  Sleep(50);
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  ptimer_alarm_fired_callback = nullptr;

  TEST_INFO("Unaligned ALARM_0 (0x201F with TIME_0=0x2000): firings = %u",
            g_alarm_capture_count);
  if (g_alarm_capture_count > 0) {
    TEST_FAIL(
        "Unaligned ALARM_0 write evaluated unmasked and fired past alarm "
        "immediately!");
    return false;
  }

  TEST_PASS("Unaligned ALARM_0 write correctly masked before comparison.");
  return true;
}
