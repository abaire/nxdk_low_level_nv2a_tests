#include "test_interrupt_acknowledge.h"

#include "ptimer_test_common.h"

bool TestInterruptAcknowledge::Test() {
  bool pass = true;

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
  VIDEOREG(NV_PTIMER_INTR_0) = 1;
  VIDEOREG(NV_PTIMER_TIME_0) = 0x00001000;
  VIDEOREG(NV_PTIMER_ALARM_0) = 0x00100000;
  while (VIDEOREG(NV_PTIMER_TIME_0) < 0x00200000) {
  }

  uint32_t p1 = VIDEOREG(NV_PTIMER_INTR_0);
  TEST_INFO("Initial pending INTR_0: 0x%08X", p1);
  if ((p1 & 1) == 0) {
    TEST_FAIL("Expected pending bit to be 1 before acknowledge test!");
    pass = false;
  }

  VIDEOREG(NV_PTIMER_INTR_0) = 0;
  uint32_t p2 = VIDEOREG(NV_PTIMER_INTR_0);
  TEST_INFO("After writing 0 to INTR_0: 0x%08X (expected 1)", p2);
  if ((p2 & 1) != 1) {
    TEST_FAIL("Writing 0 cleared pending bit!");
    pass = false;
  }

  VIDEOREG(NV_PTIMER_INTR_0) = 1;
  uint32_t p3 = VIDEOREG(NV_PTIMER_INTR_0);
  TEST_INFO("After writing 1 to INTR_0: 0x%08X (expected 0)", p3);
  if ((p3 & 1) != 0) {
    TEST_FAIL("Writing 1 did not clear pending bit!");
    pass = false;
  } else if ((p2 & 1) == 1) {
    TEST_PASS(
        "Write-1-to-Clear verified: writing 0 retains, writing 1 clears.");
  }

  return pass;
}
