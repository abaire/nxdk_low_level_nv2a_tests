#include "ptimer_test_common.h"

#include <cstdarg>
#include <cstdio>

#include "logger.h"
#include "printf/printf.h"

ClockState g_clock_state;
AlarmCapture g_alarm_captures[kMaxAlarmCaptures];
volatile size_t g_alarm_capture_count = 0;

static std::deque<std::string> g_on_screen_log;
static constexpr size_t kMaxOnScreenLogLines = 18;

void LogMsg(const char* fmt, ...) {
  char buf[512];
  va_list args;
  va_start(args, fmt);
  vsnprintf_(buf, sizeof(buf), fmt, args);
  va_end(args);

  DbgPrint("%s", buf);

  if (Logger::IsInitialized()) {
    Logger::Log() << buf;
  }

  std::string s(buf);
  while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) {
    s.pop_back();
  }
  if (!s.empty()) {
    g_on_screen_log.push_back(s);
    if (g_on_screen_log.size() > kMaxOnScreenLogLines) {
      g_on_screen_log.pop_front();
    }
  }
}

const std::deque<std::string>& GetOnScreenLog() { return g_on_screen_log; }

void ClearOnScreenLog() { g_on_screen_log.clear(); }

void CapturingAlarmCallback() {
  size_t idx = g_alarm_capture_count;
  if (idx < kMaxAlarmCaptures) {
    g_alarm_captures[idx].alarm_count = ptimer_alarm_count;
    g_alarm_captures[idx].time_0 = VIDEOREG(NV_PTIMER_TIME_0);
    g_alarm_captures[idx].time_1 = VIDEOREG(NV_PTIMER_TIME_1);
    g_alarm_captures[idx].alarm_reg = VIDEOREG(NV_PTIMER_ALARM_0);
    QueryPerformanceCounter(&g_alarm_captures[idx].qpc);
    g_alarm_captures[idx].tsc = GetTSCTime();
    g_alarm_capture_count = idx + 1;
  }
}

void ClockState::Init() {
  QueryPerformanceFrequency(&qpc_frequency);
  QueryPerformanceCounter(&last_qpc);
  last_ptimer = GetPTIMERTime();
  last_tsc = GetTSCTime();
  initialized = true;
}

double ClockState::DeltaQPCNanoseconds(int64_t ticks) const {
  if (!initialized || qpc_frequency.QuadPart == 0) {
    return 0.0;
  }
  return ticks * kNanosecondsPerSecond / qpc_frequency.QuadPart;
}

double ClockState::DeltaPTIMERNanoseconds(int64_t ticks) {
  uint32_t numerator = VIDEOREG(NV_PTIMER_NUMERATOR);
  uint32_t denominator = VIDEOREG(NV_PTIMER_DENOMINATOR);
  if (denominator == 0) {
    return 0.0;
  }
  auto dticks = static_cast<double>(ticks >> 5);
  auto gpu_ticks = dticks * numerator / denominator;
  return gpu_ticks * kNanosecondsPerSecond / kNV2ACoreFrequency;
}

void ClockState::Update(bool update_deltas) {
  ptimer_now = GetPTIMERTime();
  tsc_now = GetTSCTime();
  QueryPerformanceCounter(&qpc_now);

  if (!update_deltas) {
    return;
  }

  qpc_ticks = qpc_now.QuadPart - last_qpc.QuadPart;
  qpc_nanoseconds =
      (qpc_frequency.QuadPart > 0)
          ? static_cast<double>(qpc_ticks * kNanosecondsPerSecond) /
                qpc_frequency.QuadPart
          : 0.0;

  ptimer_ticks = ptimer_now - last_ptimer;
  tsc_ticks = tsc_now - last_tsc;

  double elapsed_sec = (qpc_frequency.QuadPart > 0)
                           ? (double)qpc_ticks / qpc_frequency.QuadPart
                           : 0.0;
  mhz = (elapsed_sec > 0.0) ? ((double)ptimer_ticks / elapsed_sec / 1000000.0)
                            : 0.0;

  ptimer_ticks_per_qpc_ticks =
      (qpc_ticks > 0) ? static_cast<double>(ptimer_ticks) / qpc_ticks : 0.0;

  last_qpc = qpc_now;
  last_ptimer = ptimer_now;
  last_tsc = tsc_now;
}
