#ifndef XBOX
#error Must be built with nxdk
#endif

#include <SDL.h>
#include <hal/debug.h>
#include <hal/video.h>
#include <pbkit/pbkit.h>
#include <windows.h>

#include <cassert>
#include <string>
#include <vector>

#include "logger.h"
#include "nv2astate.h"
#include "ptimer_tests/ptimer_test_common.h"
#include "ptimer_tests/test_alarm_adjust_preserves_pending_flag.h"
#include "ptimer_tests/test_alarm_flag_is_set_while_masked.h"
#include "ptimer_tests/test_alarm_flag_is_set_while_masked_reset_time.h"
#include "ptimer_tests/test_alarm_rearm.h"
#include "ptimer_tests/test_alarm_rollover_and_periodicity.h"
#include "ptimer_tests/test_alarm_set_in_the_past.h"
#include "ptimer_tests/test_alarm_target_zero.h"
#include "ptimer_tests/test_alarm_unaligned_target.h"
#include "ptimer_tests/test_alarm_unprogrammed.h"
#include "ptimer_tests/test_clock_scaling_ratios.h"
#include "ptimer_tests/test_interrupt_acknowledge.h"
#include "ptimer_tests/test_interrupt_masking.h"
#include "ptimer_tests/test_register_bitmasks.h"
#include "ptimer_tests/test_time_register_read_write.h"
#include "ptimer_tests/test_time_register_rollover.h"
#include "ptimer_tests/test_zero_divisor_clock.h"
#include "test_suite.h"
#include "test_util.h"

static const std::string kLogPath =
    R"(e:\devkit\nxdk_low_level_nv2a_tests\log.txt)";

static constexpr TestCase kTests[] = {
    TestCase::From<TestAlarmUnprogrammed>(),
    TestCase::From<TestAlarmTargetZero>(),
    TestCase::From<TestAlarmRearm>(),
    TestCase::From<TestAlarmAdjustPreservesPendingFlag>(),
    TestCase::From<TestAlarmRolloverAndPeriodicity>(),
    TestCase::From<TestAlarmSetInThePast>(),
    TestCase::From<TestAlarmUnalignedTarget>(),
    TestCase::From<TestRegisterBitmasks>(),
    TestCase::From<TestInterruptMasking>(),
    TestCase::From<TestAlarmFlagIsSetWhileMasked>(),
    TestCase::From<TestAlarmFlagIsSetWhileMaskedResetTime>(),
    TestCase::From<TestInterruptAcknowledge>(),
    TestCase::From<TestTimeRegisterReadWrite>(),
    TestCase::From<TestTimeRegisterRollover>(),
    TestCase::From<TestClockScalingRatios>(),
    TestCase::From<TestZeroDivisorClock>(),
};

static void RenderLogScreen() {
  pb_wait_for_vbl();
  pb_reset();
  pb_target_back_buffer();
  ClearScreen(0);
  pb_print("  NV2A PTIMER Hardware Validation Suite\n");
  pb_print("============================================================\n\n");

  const auto& log = GetOnScreenLog();
  for (const auto& line : log) {
    pb_print("%s\n", line.c_str());
  }

  pb_draw_text_screen();
  PBKitPlusPlus::NV2AState::FinishDraw();
}

static void RunAllTests() {
  std::vector<bool> results;
  results.reserve(std::size(kTests));

  for (const auto& test : kTests) {
    bool result = test.test_function();
    results.push_back(result);
    RenderLogScreen();
  }

  pb_erase_text_screen();
  ClearOnScreenLog();

  for (size_t i = 0; i < std::size(kTests); ++i) {
    LogMsg("  %2d %-28s : %s\n", i + 1, kTests[i].name,
           results[i] ? "PASS" : "FAIL");
  }
  LogMsg("See %s\n\n", kLogPath.c_str());
}

enum class AppMode {
  LOG_VIEW,
  LIVE_MONITOR,
};

int main() {
  RunPreInitBootAlarmTest();

  debugPrint("Setting video mode...\n");
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

  debugPrint("Initializing SDL Game Controller...\n");
  if (SDL_Init(SDL_INIT_GAMECONTROLLER)) {
    debugPrint("Failed to initialize SDL: %s\n", SDL_GetError());
  }

  debugPrint("Mounting Drive E: for log output...\n");
  if (EnsureDriveMounted('E', false)) {
    size_t last_slash = kLogPath.find_last_of("\\/");
    if (last_slash != std::string::npos) {
      EnsureFolderExists(kLogPath.substr(0, last_slash));
    }
    Logger::Initialize(kLogPath, true);
    LogMsg("Mounted Drive E: and initialized log at %s\n", kLogPath.c_str());
  } else {
    DbgPrint("WARNING: Failed to mount Drive E:! Disk logging disabled.\n");
  }

  g_clock_state.Init();

  VIDEOREG(NV_PTIMER_NUMERATOR) = kTestNumerator;
  VIDEOREG(NV_PTIMER_DENOMINATOR) = kTestDenominator;

  AppMode mode = AppMode::LOG_VIEW;
  bool running = true;

  pb_show_front_screen();
  debugClearScreen();

  pb_wait_for_vbl();
  pb_reset();
  pb_target_back_buffer();
  ClearScreen(0);
  pb_print("  NV2A PTIMER Hardware Validation Suite\n");
  pb_print("============================================================\n\n");
  pb_print("Testing (this may take some time)...\n");
  pb_draw_text_screen();
  PBKitPlusPlus::NV2AState::FinishDraw();

  LogMsg("Running test suite...\n");
  RunAllTests();

  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED: {
          SDL_GameControllerOpen(event.cdevice.which);
        } break;
        case SDL_CONTROLLERDEVICEREMOVED: {
          SDL_GameController* controller =
              SDL_GameControllerFromInstanceID(event.cdevice.which);
          if (controller) SDL_GameControllerClose(controller);
        } break;
        case SDL_CONTROLLERBUTTONUP: {
          if (event.cbutton.state == SDL_RELEASED) {
            auto btn =
                static_cast<SDL_GameControllerButton>(event.cbutton.button);

            if (btn == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER ||
                btn == SDL_CONTROLLER_BUTTON_START) {
              running = false;
            }

            if (mode == AppMode::LOG_VIEW) {
              if (btn == SDL_CONTROLLER_BUTTON_Y) {
                mode = AppMode::LIVE_MONITOR;
              }
            } else if (mode == AppMode::LIVE_MONITOR) {
              if (btn == SDL_CONTROLLER_BUTTON_B ||
                  btn == SDL_CONTROLLER_BUTTON_BACK) {
                mode = AppMode::LOG_VIEW;
              }
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
    ClearScreen(0);

    g_clock_state.Update();

    if (mode == AppMode::LOG_VIEW) {
      pb_print("  NV2A PTIMER Hardware Validation Suite\n");
      pb_print("Log file: %s\n\n", kLogPath.c_str());

      const auto& log = GetOnScreenLog();
      for (const auto& line : log) {
        pb_print("%s\n", line.c_str());
      }

      pb_print("\n[Y] Live Monitor      [Black] Exit\n");
    } else if (mode == AppMode::LIVE_MONITOR) {
      pb_print("  PTIMER Live Monitor Mode\n");

      uint32_t t0 = VIDEOREG(NV_PTIMER_TIME_0);
      uint32_t t1 = VIDEOREG(NV_PTIMER_TIME_1);
      uint32_t alarm = VIDEOREG(NV_PTIMER_ALARM_0);
      uint32_t intr = VIDEOREG(NV_PTIMER_INTR_0);
      uint32_t intr_en = VIDEOREG(NV_PTIMER_INTR_EN_0);
      uint32_t num = VIDEOREG(NV_PTIMER_NUMERATOR);
      uint32_t den = VIDEOREG(NV_PTIMER_DENOMINATOR);

      pb_print("PTIMER_TIME_1 : 0x%08X (High 29 bits)\n", t1);
      pb_print("PTIMER_TIME_0 : 0x%08X (Low 27 bits << 5)\n", t0);
      pb_print("PTIMER_ALARM_0: 0x%08X\n", alarm);
      pb_print("INTR_0 (Status): 0x%08X  |  INTR_EN_0 (Mask): 0x%08X\n", intr,
               intr_en);
      pb_print("NUMERATOR     : %u (0x%X) |  DENOMINATOR : %u (0x%X)\n\n", num,
               num, den, den);

      pb_print("Alarm Count   : %u\n", ptimer_alarm_count);
      pb_print("PTIMER MHz    : %.2f MHz\n", g_clock_state.mhz);
      pb_print("PTIMER Ticks  : %llu\n", g_clock_state.ptimer_ticks);
      pb_print("CPU QPC Ticks : %llu (%.3f ms)\n", g_clock_state.qpc_ticks,
               g_clock_state.qpc_nanoseconds / 1000000.0);

      pb_print("\n[B] Return to Summary\n");
      pb_print("[Black] Exit\n");
    }

    pb_draw_text_screen();
    PBKitPlusPlus::NV2AState::FinishDraw();
  }

  pb_kill();
  return 0;
}
