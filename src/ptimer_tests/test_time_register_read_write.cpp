#include "test_time_register_read_write.h"

#include "ptimer_test_common.h"

bool TestTimeRegisterReadWrite::Test() {
  bool pass = true;

  uint32_t test_time_0_values[] = {0x00001000, 0x00100000, 0x12345000,
                                   0x7F000000};
  for (uint32_t write_val : test_time_0_values) {
    VIDEOREG(NV_PTIMER_TIME_1) = 0x00000002;
    VIDEOREG(NV_PTIMER_TIME_0) = write_val;

    uint32_t r_time_0 = VIDEOREG(NV_PTIMER_TIME_0);
    uint32_t r_time_1 = VIDEOREG(NV_PTIMER_TIME_1);

    TEST_INFO("Wrote TIME_0=0x%08X (with TIME_1=2):", write_val);
    TEST_INFO("Readback: TIME_0=0x%08X, TIME_1=0x%08X", r_time_0, r_time_1);

    if (r_time_1 != 0x00000002) {
      TEST_FAIL_EX(TestFields("expected", Hex32(2), "actual", Hex32(r_time_1)),
                   "Writing TIME_0 corrupted TIME_1 (expected 2, got 0x%08X)!",
                   r_time_1);
      pass = false;
    }

    if (r_time_0 < write_val || r_time_0 > write_val + 0x20000) {
      TEST_FAIL("TIME_0 readback diverged significantly from written value!");
      pass = false;
    } else {
      TEST_PASS("TIME_0 readback matches written value (no 32x shift bug).");
    }
  }

  uint32_t test_time_1_values[] = {0x00000001, 0x00000008, 0x10000000};
  for (uint32_t write_val : test_time_1_values) {
    VIDEOREG(NV_PTIMER_TIME_0) = 0x00050000;
    VIDEOREG(NV_PTIMER_TIME_1) = write_val;

    uint32_t r_time_0 = VIDEOREG(NV_PTIMER_TIME_0);
    uint32_t r_time_1 = VIDEOREG(NV_PTIMER_TIME_1);

    TEST_INFO("Wrote TIME_1=0x%08X (with TIME_0=0x00050000):", write_val);
    TEST_INFO("Readback: TIME_1=0x%08X, TIME_0=0x%08X", r_time_1, r_time_0);

    if (r_time_1 != write_val) {
      TEST_FAIL_EX(
          TestFields("expected", Hex32(write_val), "actual", Hex32(r_time_1)),
          "TIME_1 readback mismatch (expected 0x%08X, got 0x%08X)!", write_val,
          r_time_1);
      pass = false;
    } else {
      TEST_PASS("TIME_1 readback matches written value.");
    }
  }

  return pass;
}
