#include "test_register_bitmasks.h"

#include "ptimer_test_common.h"

bool TestRegisterBitmasks::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: Register Bitmask & Alignment Validation\n");

  bool pass = true;

  VIDEOREG(NV_PTIMER_ALARM_0) = 0x1234567F;
  uint32_t alarm_read = VIDEOREG(NV_PTIMER_ALARM_0);
  LogMsg(
      "Wrote ALARM_0 = 0x1234567F, Read back = 0x%08X (expected 0x12345660)\n",
      alarm_read);
  if ((alarm_read & 0x1F) != 0) {
    LogMsg("  [FAIL] ALARM_0 lower 5 bits were not masked off!\n");
    pass = false;
  } else {
    LogMsg("  [PASS] ALARM_0 lower 5 bits are 0.\n");
  }

  VIDEOREG(NV_PTIMER_TIME_0) = 0xFFFFFFFF;
  uint32_t time_0_read = VIDEOREG(NV_PTIMER_TIME_0);
  LogMsg("Wrote TIME_0 = 0xFFFFFFFF, Read back = 0x%08X\n", time_0_read);
  if ((time_0_read & 0x1F) != 0) {
    LogMsg("  [FAIL] TIME_0 lower 5 bits were not 0!\n");
    pass = false;
  } else {
    LogMsg("  [PASS] TIME_0 lower 5 bits are 0.\n");
  }

  VIDEOREG(NV_PTIMER_TIME_1) = 0xFFFFFFFF;
  uint32_t time_1_read = VIDEOREG(NV_PTIMER_TIME_1);
  LogMsg("Wrote TIME_1 = 0xFFFFFFFF, Read back = 0x%08X (mask: 0x%08X)\n",
         time_1_read, CLOCK_HIGH_MASK);
  if ((time_1_read & ~CLOCK_HIGH_MASK) != 0) {
    LogMsg("  [FAIL] TIME_1 bits [31:29] were not 0!\n");
    pass = false;
  } else {
    LogMsg("  [PASS] TIME_1 bits [31:29] are 0.\n");
  }

  return pass;
}
