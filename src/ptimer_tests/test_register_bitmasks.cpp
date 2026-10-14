#include "test_register_bitmasks.h"

#include "ptimer_test_common.h"

bool TestRegisterBitmasks::Test() {
  bool pass = true;

  VIDEOREG(NV_PTIMER_ALARM_0) = 0x1234567F;
  uint32_t alarm_read = VIDEOREG(NV_PTIMER_ALARM_0);
  TEST_INFO(
      "Wrote ALARM_0 = 0x1234567F, Read back = 0x%08X (expected 0x12345660)",
      alarm_read);
  if ((alarm_read & 0x1F) != 0) {
    TEST_FAIL("ALARM_0 lower 5 bits were not masked off!");
    pass = false;
  } else {
    TEST_PASS("ALARM_0 lower 5 bits are 0.");
  }

  VIDEOREG(NV_PTIMER_TIME_0) = 0xFFFFFFFF;
  uint32_t time_0_read = VIDEOREG(NV_PTIMER_TIME_0);
  TEST_INFO("Wrote TIME_0 = 0xFFFFFFFF, Read back = 0x%08X", time_0_read);
  if ((time_0_read & 0x1F) != 0) {
    TEST_FAIL("TIME_0 lower 5 bits were not 0!");
    pass = false;
  } else {
    TEST_PASS("TIME_0 lower 5 bits are 0.");
  }

  VIDEOREG(NV_PTIMER_TIME_1) = 0xFFFFFFFF;
  uint32_t time_1_read = VIDEOREG(NV_PTIMER_TIME_1);
  TEST_INFO("Wrote TIME_1 = 0xFFFFFFFF, Read back = 0x%08X (mask: 0x%08X)",
            time_1_read, CLOCK_HIGH_MASK);
  if ((time_1_read & ~CLOCK_HIGH_MASK) != 0) {
    TEST_FAIL("TIME_1 bits [31:29] were not 0!");
    pass = false;
  } else {
    TEST_PASS("TIME_1 bits [31:29] are 0.");
  }

  return pass;
}
