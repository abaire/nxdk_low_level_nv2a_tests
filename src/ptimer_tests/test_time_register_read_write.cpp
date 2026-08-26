#include "test_time_register_read_write.h"

#include "ptimer_test_common.h"

bool TestTimeRegisterReadWrite::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: Time Register R/W Isolation & Scaling Check\n");

  bool pass = true;

  uint32_t test_time_0_values[] = {0x00001000, 0x00100000, 0x12345000,
                                   0x7F000000};
  for (uint32_t write_val : test_time_0_values) {
    VIDEOREG(NV_PTIMER_TIME_1) = 0x00000002;
    VIDEOREG(NV_PTIMER_TIME_0) = write_val;

    uint32_t r_time_0 = VIDEOREG(NV_PTIMER_TIME_0);
    uint32_t r_time_1 = VIDEOREG(NV_PTIMER_TIME_1);

    LogMsg("Wrote TIME_0=0x%08X (with TIME_1=2):\n", write_val);
    LogMsg("  Readback: TIME_0=0x%08X, TIME_1=0x%08X\n", r_time_0, r_time_1);

    if (r_time_1 != 0x00000002) {
      LogMsg(
          "  [FAIL] Writing TIME_0 corrupted TIME_1 (expected 2, got "
          "0x%08X)!\n",
          r_time_1);
      pass = false;
    }

    if (r_time_0 < write_val || r_time_0 > write_val + 0x20000) {
      LogMsg(
          "  [FAIL] TIME_0 readback diverged significantly from written "
          "value!\n");
      pass = false;
    } else {
      LogMsg(
          "  [PASS] TIME_0 readback matches written value (no 32x shift "
          "bug).\n");
    }
  }

  uint32_t test_time_1_values[] = {0x00000001, 0x00000008, 0x10000000};
  for (uint32_t write_val : test_time_1_values) {
    VIDEOREG(NV_PTIMER_TIME_0) = 0x00050000;
    VIDEOREG(NV_PTIMER_TIME_1) = write_val;

    uint32_t r_time_0 = VIDEOREG(NV_PTIMER_TIME_0);
    uint32_t r_time_1 = VIDEOREG(NV_PTIMER_TIME_1);

    LogMsg("Wrote TIME_1=0x%08X (with TIME_0=0x00050000):\n", write_val);
    LogMsg("  Readback: TIME_1=0x%08X, TIME_0=0x%08X\n", r_time_1, r_time_0);

    if (r_time_1 != write_val) {
      LogMsg(
          "  [FAIL] TIME_1 readback mismatch (expected 0x%08X, got 0x%08X)!\n",
          write_val, r_time_1);
      pass = false;
    } else {
      LogMsg("  [PASS] TIME_1 readback matches written value.\n");
    }
  }

  return pass;
}
