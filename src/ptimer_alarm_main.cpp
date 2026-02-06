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

// #define VERIFY_ALARM_SET_IN_THE_PAST_FIRES_IMMEDIATELY
#define VERIFY_BEHAVIOR_OF_ALARM_ROLLOVER

extern "C" volatile DWORD ptimer_alarm_count;
extern "C" void (*ptimer_alarm_fired_callback)();

static constexpr DWORD kMaxNumeratorDenominator = 0xFFFF;
// Observed values from Def Jam: Fight for NY
static constexpr DWORD kTestNumerator = 0xDE86;
static constexpr DWORD kTestDenominator = 0x1DCD;
static constexpr auto kNanosecondsPerSecond = 1000000000.0;
static constexpr auto kNanosecondsPerMillisecond = 1000000;

static constexpr uint64_t kTicksPerInterval = 16666666;  // ~60 Hz at 1GHz

static inline uint64_t GetPTIMERTime() {
  DWORD now_ptimer = VIDEOREG(NV_PTIMER_TIME_0);
  DWORD now_ptimer_1 = VIDEOREG(NV_PTIMER_TIME_1);

  return (static_cast<uint64_t>(now_ptimer_1) << 32) + now_ptimer;
}

struct ClockState {
  ClockState() {
    QueryPerformanceFrequency(&qpc_frequency);
    QueryPerformanceCounter(&last_qpc);
    last_ptimer = GetPTIMERTime();
  }

  void Update(bool update_deltas = true) {
    ptimer_now = GetPTIMERTime();
    QueryPerformanceCounter(&qpc_now);

    if (!update_deltas) {
      return;
    }

    qpc_ticks = qpc_now.QuadPart - last_qpc.QuadPart;
    qpc_nanoseconds = static_cast<double>(qpc_ticks * kNanosecondsPerSecond) /
                      qpc_frequency.QuadPart;

    ptimer_ticks = ptimer_now - last_ptimer;
    double elapsed_sec = (double)qpc_ticks / qpc_frequency.QuadPart;
    mhz = (elapsed_sec > 0) ? ((double)ptimer_ticks / elapsed_sec / 1000000.0)
                            : 0.0;

    ptimer_ticks_per_qpc_ticks = static_cast<double>(ptimer_ticks) / qpc_ticks;

    last_qpc = qpc_now;
    last_ptimer = ptimer_now;
  }

  LARGE_INTEGER qpc_frequency;
  LARGE_INTEGER last_qpc;
  uint64_t last_ptimer{0};

  LARGE_INTEGER qpc_now;
  uint64_t ptimer_now{0};

  uint64_t ptimer_ticks{0};
  uint64_t qpc_ticks{0};
  uint64_t qpc_nanoseconds{0};
  double mhz{0.0};
  double ptimer_ticks_per_qpc_ticks{0.0};
};

static uint64_t next_alarm_time = 0x00FFFFFF;
static uint32_t alarms_scheduled = 1;

#ifndef VERIFY_BEHAVIOR_OF_ALARM_ROLLOVER
static void OnAlarmFired() {
  next_alarm_time += kTicksPerInterval;
  VIDEOREG(NV_PTIMER_ALARM_0) = static_cast<uint32_t>(next_alarm_time);
  alarms_scheduled++;
}
#endif

#ifdef VERIFY_BEHAVIOR_OF_ALARM_ROLLOVER
static void TestTenAlarmCycles() {
  ClockState clock_state;

  ptimer_alarm_count = 0;

  static constexpr auto kAlarmScheduleMilliseconds = 32;
  static constexpr auto kAlarmScheduleNanoseconds =
      kAlarmScheduleMilliseconds * kNanosecondsPerMillisecond;

  // Force the clock to a known starting point that will require wraparound.
  static constexpr auto kTestTimerStartHigh = 5;
  clock_state.ptimer_now = (static_cast<uint64_t>(kTestTimerStartHigh) << 32) +
                           (0xFFFFFFFF - (kAlarmScheduleNanoseconds / 2));

  // Schedule an alarm
  uint32_t time_1 = clock_state.ptimer_now >> 32;
  VIDEOREG(NV_PTIMER_TIME_1) = time_1;
  VIDEOREG(NV_PTIMER_TIME_0) = clock_state.ptimer_now & 0xFFFFFFFF;

  const uint32_t alarm_time =
      clock_state.ptimer_now + kAlarmScheduleNanoseconds;
  VIDEOREG(NV_PTIMER_ALARM_0) = alarm_time;

  DbgPrint("PTIMER clock setup: 0x%X / 0x%X\n", VIDEOREG(NV_PTIMER_NUMERATOR),
           VIDEOREG(NV_PTIMER_DENOMINATOR));
  DbgPrint(
      "Current time is 0x%X 0x%X, alarm at 0x%X (init 0x%X, 0x%X delta) alarm "
      "count "
      "%d\n",
      VIDEOREG(NV_PTIMER_TIME_1), VIDEOREG(NV_PTIMER_TIME_0),
      VIDEOREG(NV_PTIMER_ALARM_0), alarm_time,
      0xFFFFFFFF - static_cast<uint32_t>(clock_state.ptimer_now) + alarm_time,
      ptimer_alarm_count);

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;

  uint32_t last_alarm_count = ptimer_alarm_count;
  uint32_t last_alarm_val = 0;
  uint32_t last_alarm_time_0 = 0;
  uint32_t last_alarm_time_1 = 0;

  while (ptimer_alarm_count < 10) {
    if (ptimer_alarm_count == last_alarm_count) {
      continue;
    }

    uint32_t now_0 = VIDEOREG(NV_PTIMER_TIME_0);
    uint32_t now_1 = VIDEOREG(NV_PTIMER_TIME_1);
    uint32_t now_alarm = VIDEOREG(NV_PTIMER_ALARM_0);

    DbgPrint("  > Alarm count %d  time 0x%X 0x%X alarm reg: 0x%X init 0x%X\n",
             ptimer_alarm_count, now_1, now_0, now_alarm, alarm_time);
    if (last_alarm_val || last_alarm_time_0 || last_alarm_time_1) {
      DbgPrint("    Delta high: 0x%X low: 0x%X alarm: 0x%X\n",
               now_1 - last_alarm_time_1, now_0 - last_alarm_time_0,
               now_alarm - last_alarm_val);
    }

    last_alarm_count = ptimer_alarm_count;
    last_alarm_val = now_alarm;
    last_alarm_time_0 = now_0;
    last_alarm_time_1 = now_1;
  }
  auto end_time_1 = VIDEOREG(NV_PTIMER_TIME_1);
  auto end_time_0 = VIDEOREG(NV_PTIMER_TIME_0);

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;

  DbgPrint("  ! Alarm count %d  time 0x%X 0x%X. Was scheduled with 0x%X 0x%X\n",
           ptimer_alarm_count, end_time_1, end_time_0, kTestTimerStartHigh,
           alarm_time);

  if (VIDEOREG(NV_PTIMER_ALARM_0) != alarm_time) {
    DbgPrint("!!! ALARM_0 was modified: 0x%X != scheduled 0x%X\n",
             VIDEOREG(NV_PTIMER_ALARM_0), alarm_time);
  }
}

static void TestAlarmRolloverAndAutoReset() {
  DWORD ptimer_numerator = kTestNumerator;
  DWORD ptimer_denominator = kTestDenominator;
  VIDEOREG(NV_PTIMER_NUMERATOR) = ptimer_numerator;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = ptimer_denominator;
  TestTenAlarmCycles();

  VIDEOREG(NV_PTIMER_NUMERATOR) = 1000;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = 100;
  TestTenAlarmCycles();
}
#endif

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

#ifdef VERIFY_BEHAVIOR_OF_ALARM_ROLLOVER
  TestAlarmRolloverAndAutoReset();
  Sleep(1000);
  pb_kill();
  return 0;
#else

  ClockState clock_state;

#ifdef VERIFY_ALARM_SET_IN_THE_PAST_FIRES_IMMEDIATELY
  VIDEOREG(NV_PTIMER_TIME_1) = 1;
  VIDEOREG(NV_PTIMER_ALARM_0) = clock_state.last_ptimer - 1;
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;

  Sleep(50);  // Wait for potential interrupt

  VIDEOREG(NV_PTIMER_INTR_EN_0) = 0;
#else
  ptimer_alarm_fired_callback = OnAlarmFired;

  VIDEOREG(NV_PTIMER_TIME_1) = 1;
  VIDEOREG(NV_PTIMER_ALARM_0) =
      (clock_state.last_ptimer + 100000000) & 0xFFFFFFFF;
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;
#endif  // VERIFY_ALARM_SET_IN_THE_PAST_FIRES_IMMEDIATELY

  DWORD ptimer_numerator = kTestNumerator;
  DWORD ptimer_denominator = kTestDenominator;
  VIDEOREG(NV_PTIMER_NUMERATOR) = ptimer_numerator;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = ptimer_denominator;

  bool running = true;
  bool freeze_tick_display = false;

  // Alarm Test State
  uint32_t initial_alarm_count = ptimer_alarm_count;

  // Prime the first alarm
  VIDEOREG(NV_PTIMER_ALARM_0) = static_cast<uint32_t>(next_alarm_time);

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
                // Reset Test
                initial_alarm_count = ptimer_alarm_count;
                alarms_scheduled = 1;
                clock_state.Update();
                next_alarm_time = clock_state.ptimer_now + kTicksPerInterval;
                VIDEOREG(NV_PTIMER_ALARM_0) =
                    static_cast<uint32_t>(next_alarm_time);
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

    PBKitBusyWait();

    clock_state.Update(!freeze_tick_display);

    // Check/Update Alarm
    uint32_t actual_alarms = ptimer_alarm_count - initial_alarm_count;

    pb_print("PTIMER Test\n");
    pb_print("-----------\n");
    pb_print("Alarm Reg:   0x%X\n", VIDEOREG(NV_PTIMER_ALARM_0));
    pb_print("Alarm Count: %d\n", ptimer_alarm_count);
    pb_print("Time 0:      0x%X\n", (clock_state.ptimer_now & 0xFFFFFFFF));
    pb_print("Time 1:      0x%X\n", (clock_state.ptimer_now >> 32));
    pb_print("\n");
    pb_print("Scaling Validation (Num=%d, Den=%d %f)\n", ptimer_numerator,
             ptimer_denominator,
             static_cast<float>(ptimer_numerator) / ptimer_denominator);
    pb_print("GPU Ticks: %15llu - rate: %.2f MHz\n", clock_state.ptimer_ticks,
             clock_state.mhz);
    pb_print("CPU ticks: %15llu - nanos: %10llu\n", clock_state.qpc_ticks,
             clock_state.qpc_nanoseconds);
    pb_print("GPU / CPU factor: %3f\n", clock_state.ptimer_ticks_per_qpc_ticks);
    pb_print("\n");

#if !(defined(VERIFY_BEHAVIOR_OF_ALARM_ROLLOVER) || \
      defined(VERIFY_ALARM_SET_IN_THE_PAST_FIRES_IMMEDIATELY))
    pb_print("Drift Test:\n");
    pb_print("Scheduled: %d\n", alarms_scheduled);
    pb_print("Fired:     %d\n", actual_alarms);
    pb_print("Diff:      %d\n", (int)(alarms_scheduled - actual_alarms));
#endif

    pb_draw_text_screen();

    PBKitBusyWait();
    PBKitFlip();
  }

  pb_kill();
  return 0;
#endif
}
