#ifndef XBOX
#error Must be built with nxdk
#endif

#include <SDL.h>
#include <hal/debug.h>
#include <hal/fileio.h>
#include <hal/video.h>
#include <nxdk/format.h>
#include <nxdk/mount.h>
#include <pbkit/pbkit.h>
#include <windows.h>

#include <cassert>
#include <cstring>
#include <string>
#include <vector>

#include "logger.h"
#include "m2mf_tests/m2mf_test_common.h"
#include "m2mf_tests/test_m2mf_alignment_odd_sizes.h"
#include "m2mf_tests/test_m2mf_alignment_phases.h"
#include "m2mf_tests/test_m2mf_batching_causal_chain.h"
#include "m2mf_tests/test_m2mf_batching_independent.h"
#include "m2mf_tests/test_m2mf_format.h"
#include "m2mf_tests/test_m2mf_framebuffer.h"
#include "m2mf_tests/test_m2mf_large_pitch.h"
#include "m2mf_tests/test_m2mf_linear_copy.h"
#include "m2mf_tests/test_m2mf_max_line_count.h"
#include "m2mf_tests/test_m2mf_min_transfer.h"
#include "m2mf_tests/test_m2mf_notifier_record.h"
#include "m2mf_tests/test_m2mf_notifier_switching.h"
#include "m2mf_tests/test_m2mf_pitched_asymmetric.h"
#include "m2mf_tests/test_m2mf_pitched_packing.h"
#include "m2mf_tests/test_m2mf_pitched_symmetric.h"
#include "m2mf_tests/test_m2mf_sysmem_to_vram.h"
#include "m2mf_tests/test_m2mf_vram_to_sysmem.h"
#include "m2mf_tests/test_m2mf_vram_to_vram.h"
#include "m2mf_tests/test_m2mf_zero_size.h"
#include "nv2astate.h"
#include "test_suite.h"
#include "test_util.h"

static const std::string kLogPath =
    R"(e:\devkit\nxdk_low_level_nv2a_tests\m2mf_log.txt)";

static constexpr TestCase kTests[] = {
    TestCase::From<TestM2MFAlignmentOddSizes>(),
    TestCase::From<TestM2MFAlignmentPhases>(),
    TestCase::From<TestM2MFBatchingCausalChain>(),
    TestCase::From<TestM2MFBatchingIndependent>(),
    TestCase::From<TestM2MFFormat>(),
    TestCase::From<TestM2MFFramebuffer>(),
    TestCase::From<TestM2MFLargePitch>(),
    TestCase::From<TestM2MFLinearCopy>(),
    TestCase::From<TestM2MFMaxLineCount>(),
    TestCase::From<TestM2MFMinTransfer>(),
    TestCase::From<TestM2MFNotifierRecord>(),
    TestCase::From<TestM2MFNotifierSwitching>(),
    TestCase::From<TestM2MFPitchedAsymmetric>(),
    TestCase::From<TestM2MFPitchedPacking>(),
    TestCase::From<TestM2MFPitchedSymmetric>(),
    TestCase::From<TestM2MFSysmemToVram>(),
    TestCase::From<TestM2MFVramToSysmem>(),
    TestCase::From<TestM2MFVramToVram>(),
    TestCase::From<TestM2MFZeroSize>(),
};

static void RenderLogScreen() {
  pb_wait_for_vbl();
  pb_reset();
  pb_target_back_buffer();
  ClearScreen(0);
  pb_print("  NV2A Class 0x39 (M2MF) Hardware Validation Suite\n");
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

  LogMsg("Summary:\n");
  for (size_t i = 0; i < std::size(kTests); ++i) {
    LogMsg("  %2d %-28s : %s\n", i + 1, kTests[i].name,
           results[i] ? "PASS" : "FAIL");
  }
  LogMsg("\nSee %s\n\n", kLogPath.c_str());
}

int main() {
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

  pb_show_front_screen();
  debugClearScreen();

  pb_wait_for_vbl();
  pb_reset();
  pb_target_back_buffer();
  ClearScreen(0);
  pb_print("  NV2A Class 0x39 (M2MF) Hardware Validation Suite\n");
  pb_print("============================================================\n\n");
  pb_print("Testing (this may take some time)...\n");
  pb_draw_text_screen();
  PBKitPlusPlus::NV2AState::FinishDraw();

  LogMsg("Running test suite...\n");
  RunAllTests();

  bool running = true;
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
                btn == SDL_CONTROLLER_BUTTON_START ||
                btn == SDL_CONTROLLER_BUTTON_B) {
              running = false;
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

    pb_print("  NV2A Class 0x39 (M2MF) Hardware Validation Suite\n");
    pb_print("Log file: %s\n\n", kLogPath.c_str());

    const auto& log = GetOnScreenLog();
    for (const auto& line : log) {
      pb_print("%s\n", line.c_str());
    }

    pb_print("\n[Start / B / RShoulder] Exit\n");

    pb_draw_text_screen();
    PBKitPlusPlus::NV2AState::FinishDraw();
  }

  M2MFTeardown();
  pb_kill();
  return 0;
}
