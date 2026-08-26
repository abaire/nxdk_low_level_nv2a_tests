#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_PTIMER_TEST_COMMON_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_PTIMER_TEST_COMMON_H

#include <hal/debug.h>
#include <pbkit/pbkit.h>
#include <windows.h>

#include <cstdint>
#include <deque>
#include <string>

extern "C" volatile DWORD ptimer_alarm_count;
extern "C" void (*ptimer_alarm_fired_callback)(void);

static constexpr DWORD kMaxNumeratorDenominator = 0xFFFF;
// Observed values from Def Jam: Fight for NY - 7.467033687246035
static constexpr DWORD kTestNumerator = 0xDE86;    // 56,966
static constexpr DWORD kTestDenominator = 0x1DCD;  // 7,629

static constexpr auto kNanosecondsPerSecond = 1000000000.0;
static constexpr auto kNanosecondsPerMillisecond = 1000000.0;
static constexpr auto kNanosecondsPerMicrosecond = 1000.0;
static constexpr uint64_t kNV2ACoreFrequency = 233333324;
static constexpr uint64_t kTicksPerInterval = 16666666;  // ~60 Hz at 1GHz

static constexpr uint32_t CLOCK_HIGH_MASK = 0x1FFFFFFF;
static constexpr uint32_t CLOCK_LOW_MASK = 0x07FFFFFF;
static constexpr uint32_t ALARM_MASK = 0xFFFFFFE0;

static inline uint64_t GetPTIMERTime() {
  uint32_t hi1, hi2, lo;
  do {
    hi1 = VIDEOREG(NV_PTIMER_TIME_1);
    lo = VIDEOREG(NV_PTIMER_TIME_0);
    hi2 = VIDEOREG(NV_PTIMER_TIME_1);
  } while (hi1 != hi2);
  return (static_cast<uint64_t>(hi1) << 32) | lo;
}

static inline uint64_t GetTSCTime() {
  uint32_t lo, hi;
  __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}

void LogMsg(const char* fmt, ...);
const std::deque<std::string>& GetOnScreenLog();
void ClearOnScreenLog();

struct AlarmCapture {
  uint32_t alarm_count;
  uint32_t time_0;
  uint32_t time_1;
  uint32_t alarm_reg;
  LARGE_INTEGER qpc;
  uint64_t tsc;
};

static constexpr size_t kMaxAlarmCaptures = 32;
extern AlarmCapture g_alarm_captures[kMaxAlarmCaptures];
extern volatile size_t g_alarm_capture_count;

void CapturingAlarmCallback();

struct ClockState {
  void Init();
  void Update(bool update_deltas = true);

  [[nodiscard]] double DeltaQPCNanoseconds(int64_t ticks) const;
  static double DeltaPTIMERNanoseconds(int64_t ticks);

  bool initialized{false};
  LARGE_INTEGER qpc_frequency{};
  LARGE_INTEGER last_qpc{};
  uint64_t last_ptimer{0};
  uint64_t last_tsc{0};

  LARGE_INTEGER qpc_now{};
  uint64_t ptimer_now{0};
  uint64_t tsc_now{0};

  uint64_t ptimer_ticks{0};
  uint64_t qpc_ticks{0};
  uint64_t tsc_ticks{0};
  double qpc_nanoseconds{0.0};
  double mhz{0.0};
  double ptimer_ticks_per_qpc_ticks{0.0};
};

extern ClockState g_clock_state;

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_PTIMER_TEST_COMMON_H
