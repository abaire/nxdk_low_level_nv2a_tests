#include "test_clock_scaling_ratios.h"

#include <cmath>

#include "ptimer_test_common.h"

bool TestClockScalingRatios::Test() {
  LogMsg("\n============================================================\n");
  LogMsg("Test: Clock Scaling & Frequency Measurement\n");

  struct RatioTest {
    DWORD num;
    DWORD den;
    const char* desc;
    double expected_mhz;
  };

  // PTIMER is calibrated to 1000.0 MHz (1ns/tick) at
  // kTestNumerator/kTestDenominator. Frequency = 1000.0 * (kTestNumerator /
  // kTestDenominator) * (DENOMINATOR / NUMERATOR)
  static constexpr double kBaseConstant =
      1000.0 * ((double)kTestNumerator / (double)kTestDenominator);

  RatioTest ratios[] = {
      {kTestNumerator, kTestDenominator, "Standard Xbox ratio (56966/7629)",
       1000.0},
      {1000, 1000, "1:1 ratio (1000/1000)", kBaseConstant * 1.0},
      {2000, 1000, "2:1 ratio (2000/1000)", kBaseConstant * 0.5},
  };

  LARGE_INTEGER qpc_freq;
  QueryPerformanceFrequency(&qpc_freq);

  bool pass = true;
  for (const auto& r : ratios) {
    VIDEOREG(NV_PTIMER_NUMERATOR) = r.num;
    VIDEOREG(NV_PTIMER_DENOMINATOR) = r.den;

    LARGE_INTEGER qpc_start, qpc_end;
    uint64_t ptimer_start = GetPTIMERTime();
    QueryPerformanceCounter(&qpc_start);

    Sleep(50);

    QueryPerformanceCounter(&qpc_end);
    uint64_t ptimer_end = GetPTIMERTime();

    uint64_t ptimer_delta = ptimer_end - ptimer_start;
    uint64_t qpc_delta = qpc_end.QuadPart - qpc_start.QuadPart;
    double elapsed_sec =
        static_cast<double>(qpc_delta) / static_cast<double>(qpc_freq.QuadPart);
    double measured_mhz = (elapsed_sec > 0.0)
                              ? ((double)ptimer_delta / elapsed_sec / 1000000.0)
                              : 0.0;

    LogMsg("Ratio: %s (Num=%u, Den=%u)\n", r.desc, r.num, r.den);
    LogMsg(
        "  Measured: %.2f MHz | Expected: %.2f MHz | PTIMER delta: %llu ticks "
        "(%.3f ms)\n",
        measured_mhz, r.expected_mhz, ptimer_delta, elapsed_sec * 1000.0);

    double percent_error =
        std::abs(measured_mhz - r.expected_mhz) / r.expected_mhz * 100.0;
    if (percent_error > 5.0) {
      LogMsg("  [FAIL] Measured clock frequency deviated by >5%% (%.2f%%)\n",
             percent_error);
      pass = false;
    } else {
      LogMsg("  [PASS] Measured clock frequency matches hardware model.\n");
    }
  }

  VIDEOREG(NV_PTIMER_NUMERATOR) = kTestNumerator;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = kTestDenominator;

  return pass;
}
