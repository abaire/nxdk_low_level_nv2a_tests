#ifndef XBOX
#error Must be built with nxdk
#endif

#include <SDL.h>
#include <hal/debug.h>
#include <hal/video.h>
#include <pbkit/pbkit.h>
#include <windows.h>

#include <algorithm>

#include "pbkit_util.h"

extern "C" void (*ptimer_alarm_fired_callback)();

// Observed values from Def Jam: Fight for NY
static constexpr DWORD kTestNumerator = 0xDE86;
static constexpr DWORD kTestDenominator = 0x1DCD;

static constexpr DWORD kMaxNumeratorDenominator = 0xFFFF;
static constexpr auto kNanosecondsPerSecond = 1000000000.0;
static constexpr auto kNanosecondsPerMillisecond = 1000000;

static constexpr uint64_t kTicksPerInterval = 16666666;  // ~60 Hz at 1GHz

static constexpr uint32_t kTimerSkewTestSeconds = 10;

static inline uint64_t GetPTIMERTime() {
  DWORD now_ptimer = VIDEOREG(NV_PTIMER_TIME_0);
  DWORD now_ptimer_1 = VIDEOREG(NV_PTIMER_TIME_1);

  return (static_cast<uint64_t>(now_ptimer_1) << 32) + now_ptimer;
}

static inline uint64_t GetTSCTime() {
  uint32_t lo, hi;
  __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}

struct ClockState {
  ClockState() {
    QueryPerformanceFrequency(&qpc_frequency);
    QueryPerformanceCounter(&last_qpc);
    last_ptimer = GetPTIMERTime();
    last_tsc = GetTSCTime();
  }

  void Update(bool update_deltas = true) {
    ptimer_now = GetPTIMERTime();
    tsc_now = GetTSCTime();

    QueryPerformanceCounter(&qpc_now);

    if (!update_deltas) {
      return;
    }

    qpc_ticks = qpc_now.QuadPart - last_qpc.QuadPart;
    qpc_nanoseconds = static_cast<double>(qpc_ticks * kNanosecondsPerSecond) /
                      qpc_frequency.QuadPart;

    ptimer_ticks = ptimer_now - last_ptimer;
    tsc_ticks = tsc_now - last_tsc;

    double elapsed_sec = (double)qpc_ticks / qpc_frequency.QuadPart;
    mhz = (elapsed_sec > 0) ? ((double)ptimer_ticks / elapsed_sec / 1000000.0)
                            : 0.0;

    ptimer_ticks_per_qpc_ticks = static_cast<double>(ptimer_ticks) / qpc_ticks;
    tsc_ticks_per_qpc_ticks = static_cast<double>(tsc_ticks) / qpc_ticks;

    last_qpc = qpc_now;
    last_ptimer = ptimer_now;
    last_tsc = tsc_now;
  }

  LARGE_INTEGER qpc_frequency;
  LARGE_INTEGER last_qpc;
  uint64_t last_ptimer{0};
  uint64_t last_tsc{0};

  LARGE_INTEGER qpc_now;
  uint64_t ptimer_now{0};
  uint64_t tsc_now{0};

  uint64_t ptimer_ticks{0};
  uint64_t qpc_ticks{0};
  uint64_t tsc_ticks{0};
  uint64_t qpc_nanoseconds{0};
  double mhz{0.0};
  double ptimer_ticks_per_qpc_ticks{0.0};
  double tsc_ticks_per_qpc_ticks{0.0};
};

static uint64_t last_alarm_ptimer_time;
static uint64_t last_alarm_tsc_time;
static uint64_t last_alarm_time_delta = 0;
static uint64_t next_alarm_time_ptimer = 0;

static void OnAlarmFired() {
  last_alarm_ptimer_time = GetPTIMERTime();
  last_alarm_tsc_time = GetTSCTime();
  last_alarm_time_delta = last_alarm_ptimer_time - next_alarm_time_ptimer;

  next_alarm_time_ptimer = last_alarm_ptimer_time + kTicksPerInterval;
  VIDEOREG(NV_PTIMER_ALARM_0) = static_cast<uint32_t>(next_alarm_time_ptimer);
}

struct ClockSkewTestState {
  void Start(const LARGE_INTEGER& qpc_frequency,
             uint32_t test_seconds = kTimerSkewTestSeconds) {
    QueryPerformanceCounter(&qpc_at_start);
    tsc_at_start = GetTSCTime();
    ptimer_at_start = GetPTIMERTime();
    qpc_end_target =
        qpc_at_start.QuadPart + qpc_frequency.QuadPart * test_seconds;
    qpc_at_end = 0;
    tsc_at_end = 0;
    ptimer_at_end = 0;
  }

  bool Update() {
    if (qpc_at_end) {
      return true;
    }

    LARGE_INTEGER qpc_now;
    QueryPerformanceCounter(&qpc_now);
    if (qpc_now.QuadPart >= qpc_end_target) {
      tsc_at_end = GetTSCTime();
      ptimer_at_end = GetPTIMERTime();
      qpc_at_end = qpc_now.QuadPart;
      return true;
    }

    return false;
  }

  [[nodiscard]] uint64_t QPCDelta() const {
    return qpc_at_end - qpc_at_start.QuadPart;
  }
  [[nodiscard]] uint64_t TSCDelta() const { return tsc_at_end - tsc_at_start; }
  [[nodiscard]] uint64_t PTIMERDelta() const {
    return ptimer_at_end - ptimer_at_start;
  }

  LARGE_INTEGER qpc_at_start{};
  uint64_t tsc_at_start{0};
  uint64_t ptimer_at_start{0};
  uint64_t qpc_end_target{0};

  uint64_t tsc_at_end{0};
  uint64_t ptimer_at_end{0};
  uint64_t qpc_at_end{0};
};

int main() {
  debugPrint("Set video mode");
  if (!XVideoSetMode(kFramebufferWidth, kFramebufferHeight, kBitsPerPixel,
                     REFRESH_DEFAULT)) {
    debugPrint("Failed to set video mode\n");
    Sleep(2000);
    return 1;
  }

  int status = pb_init();
  if (status) {
    debugPrint("pb_init Error %d\n", status);
    Sleep(2000);
    return 1;
  }

  debugPrint("Initializing...");
  pb_show_debug_screen();

  if (SDL_Init(SDL_INIT_GAMECONTROLLER)) {
    debugPrint("Failed to initialize SDL_GAMECONTROLLER.");
    debugPrint("%s", SDL_GetError());
    pb_show_debug_screen();
    Sleep(2000);
    return 1;
  }

  pb_show_front_screen();
  debugClearScreen();

  ClockState clock_state;

  VIDEOREG(NV_PTIMER_TIME_1) = 1;

  DWORD ptimer_numerator = kTestNumerator;
  DWORD ptimer_denominator = kTestDenominator;
  VIDEOREG(NV_PTIMER_NUMERATOR) = ptimer_numerator;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = ptimer_denominator;

  ptimer_alarm_fired_callback = OnAlarmFired;
  VIDEOREG(NV_PTIMER_TIME_1) = 1;
  VIDEOREG(NV_PTIMER_ALARM_0) =
      (clock_state.last_ptimer + 100000000) & 0xFFFFFFFF;
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;

  bool running = true;
  bool freeze_tick_display = false;

  uint32_t frame_counter = 0;
  uint64_t ptimer_peak_delta = 0;
  uint64_t qpc_peak_delta = 0;
  uint64_t tsc_peak_delta = 0;
  uint64_t alarm_peak_delta = 0;

  ClockSkewTestState skew_test_state;
  skew_test_state.Start(clock_state.qpc_frequency);

  while (running) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED: {
          SDL_GameController* controller =
              SDL_GameControllerOpen(event.cdevice.which);
          if (!controller) {
            debugPrint("Failed to handle controller add event.");
            debugPrint("%s", SDL_GetError());
            running = false;
          }
        } break;

        case SDL_CONTROLLERDEVICEREMOVED: {
          SDL_GameController* controller =
              SDL_GameControllerFromInstanceID(event.cdevice.which);
          SDL_GameControllerClose(controller);
        } break;

        case SDL_CONTROLLERBUTTONUP: {
          auto& button = event.cbutton;
          if (button.state == SDL_RELEASED) {
            switch (static_cast<SDL_GameControllerButton>(button.button)) {
              case SDL_CONTROLLER_BUTTON_DPAD_UP:
                ptimer_numerator =
                    std::min(ptimer_numerator * 10, kMaxNumeratorDenominator);
                VIDEOREG(NV_PTIMER_NUMERATOR) = ptimer_numerator;
                break;

              case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                ptimer_numerator = std::max(ptimer_numerator / 10, 1UL);
                VIDEOREG(NV_PTIMER_NUMERATOR) = ptimer_numerator;
                break;

              case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                ptimer_denominator = std::max(ptimer_denominator / 10, 1UL);
                VIDEOREG(NV_PTIMER_DENOMINATOR) = ptimer_denominator;
                break;

              case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                ptimer_denominator =
                    std::min(ptimer_denominator * 10, kMaxNumeratorDenominator);
                VIDEOREG(NV_PTIMER_DENOMINATOR) = ptimer_denominator;
                break;

              case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                running = false;
                break;

              case SDL_CONTROLLER_BUTTON_A:
                freeze_tick_display = !freeze_tick_display;
                break;

              case SDL_CONTROLLER_BUTTON_X:
                // Observed values from Def Jam: Fight for NY
                ptimer_numerator = 0xDE86;
                ptimer_denominator = 0x1DCD;
                VIDEOREG(NV_PTIMER_NUMERATOR) = ptimer_numerator;
                VIDEOREG(NV_PTIMER_DENOMINATOR) = ptimer_denominator;
                break;

              case SDL_CONTROLLER_BUTTON_Y:
                frame_counter = 0;
                ptimer_peak_delta = 0;
                qpc_peak_delta = 0;
                tsc_peak_delta = 0;
                alarm_peak_delta = 0;

                skew_test_state.Start(clock_state.qpc_frequency, 60);
                break;

              default:
                break;
            }
          }
        } break;

        default:
          break;
      }
    }

    pb_wait_for_vbl();
    pb_reset();
    pb_target_back_buffer();

    PBKitClearScreen(0);

#if 0
    // Push a specific command sequence that xemu can use to delay in order to
    // see the effect on the various clocks.
    // Note that this will trigger an exception on HW
    {
      auto p = pb_begin();
      p = pb_push1(p, NV097_WAIT_FOR_IDLE, frame_counter++);
      pb_end(p);
    }
#endif

    PBKitBusyWait();

    clock_state.Update(!freeze_tick_display);

    pb_print("Time Test %lu\n", frame_counter);
    pb_print("PTIMER Delta : %15llu N: 0x%08X 0x%08X\n",
             clock_state.ptimer_ticks,
             static_cast<uint32_t>(clock_state.ptimer_now >> 32),
             static_cast<uint32_t>(clock_state.ptimer_now));
    pb_print("TSC Delta    : %15llu N: 0x%08X 0x%08X\n", clock_state.tsc_ticks,
             static_cast<uint32_t>(clock_state.tsc_now >> 32),
             static_cast<uint32_t>(clock_state.tsc_now));
    pb_print("QPC Delta    : %15llu\n", clock_state.qpc_ticks);

    pb_print("Alarm PTIMER : %15llu\n", last_alarm_ptimer_time);
    pb_print("Alarm TSC    : %15llu\n", last_alarm_tsc_time);
    pb_print("Alarm dPTIMER: %15llu\n", last_alarm_time_delta);

    ptimer_peak_delta = std::max(ptimer_peak_delta, clock_state.ptimer_ticks);
    qpc_peak_delta = std::max(qpc_peak_delta, clock_state.qpc_ticks);
    tsc_peak_delta = std::max(tsc_peak_delta, clock_state.tsc_ticks);
    alarm_peak_delta = std::max(alarm_peak_delta, last_alarm_time_delta);

    if (frame_counter > 10) {
      pb_print(
          "PTIMER Peak:   %15llu (%f ms)\n", ptimer_peak_delta,
          static_cast<double>(ptimer_peak_delta) / kNanosecondsPerMillisecond);
      pb_print(
          "TSC Peak   :   %15llu (%f ms)\n", tsc_peak_delta,
          static_cast<double>(tsc_peak_delta) / kNanosecondsPerMillisecond);
      pb_print(
          "QPC Peak   :   %15llu (%f ms)\n", qpc_peak_delta,
          static_cast<double>(qpc_peak_delta) / kNanosecondsPerMillisecond);
      pb_print(
          "Alarm Peak :   %15llu (%f ms)\n", alarm_peak_delta,
          static_cast<double>(alarm_peak_delta) / kNanosecondsPerMillisecond);
    } else {
      ptimer_peak_delta = 0;
      qpc_peak_delta = 0;
      tsc_peak_delta = 0;
      alarm_peak_delta = 0;
    }

    if (skew_test_state.Update()) {
      pb_print("QPC at end   : %15llu\n", skew_test_state.QPCDelta());
      pb_print("TSC at end   : %15llu\n", skew_test_state.TSCDelta());
      pb_print("PTIMER at end: %15llu\n", skew_test_state.PTIMERDelta());
    } else {
      pb_print("Skew test running...\n");
    }

    pb_draw_text_screen();

    PBKitBusyWait();
    PBKitFlip();
  }

  pb_kill();
  return 0;
}
