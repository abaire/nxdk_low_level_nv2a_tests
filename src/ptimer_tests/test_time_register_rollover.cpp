#include "test_time_register_rollover.h"

#include "ptimer_test_common.h"

bool TestTimeRegisterRollover::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: Time Register 64-bit Carry & Rollover\n");

  VIDEOREG(NV_PTIMER_TIME_1) = 0x00000007;
  VIDEOREG(NV_PTIMER_TIME_0) = 0xFFFFFF00;

  LogMsg(
      "Rollover test: Starting at TIME_1=0x7, TIME_0=0xFFFFFF00, waiting for "
      "wrap...\n");
  while (VIDEOREG(NV_PTIMER_TIME_0) >= 0xFFFFFF00) {
  }

  uint32_t wrapped_time_1 = VIDEOREG(NV_PTIMER_TIME_1);
  uint32_t wrapped_time_0 = VIDEOREG(NV_PTIMER_TIME_0);
  LogMsg("  After wrap: TIME_1=0x%08X, TIME_0=0x%08X (expected TIME_1 = 0x8)\n",
         wrapped_time_1, wrapped_time_0);

  if (wrapped_time_1 != 0x00000008) {
    LogMsg("  [FAIL] TIME_1 did not increment to 0x8 after rollover!\n");
    return false;
  }

  LogMsg("  [PASS] Rollover carry propagated to TIME_1 correctly.\n");
  return true;
}
