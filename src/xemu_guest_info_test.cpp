#ifndef XBOX
#error Must be built with nxdk
#endif

#include "xemu_guest_info.h"

#include <SDL.h>
#include <hal/debug.h>
#include <hal/video.h>
#include <pbkit/pbkit.h>
#include <windows.h>

#include <string>

#include "logger.h"
#include "nv2astate.h"
#include "printf/printf.h"
#include "test_util.h"

static const std::string kLogPath =
    R"(e:\devkit\nxdk_low_level_nv2a_tests\xemu_guest_info_log.txt)";

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

static const std::deque<std::string>& GetOnScreenLog() {
  return g_on_screen_log;
}

static void RenderLogScreen() {
  pb_wait_for_vbl();
  pb_reset();
  pb_target_back_buffer();
  ClearScreen(0);
  pb_print("  xemu Guest Info Tests\n");
  pb_print("============================================================\n\n");

  const auto& log = GetOnScreenLog();
  for (const auto& line : log) {
    pb_print("%s\n", line.c_str());
  }

  pb_draw_text_screen();
  PBKitPlusPlus::NV2AState::FinishDraw();
}

static void RunAllTests() {
  auto is_xemu = xemuinfo_host_is_xemu();
  LogMsg("xemu host check: %s\n", is_xemu ? "TRUE" : "FALSE");

  XemuVersion ver{0};
  if (!xemuinfo_get_version(&ver)) {
    LogMsg("xemu version fetch failed\n");
  } else {
    LogMsg("xemu version: %d.%d.%d\n", ver.major, ver.minor, ver.patch);
  }
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
  pb_print("  xemu Guest Info Tests\n");
  pb_print("============================================================\n\n");
  pb_draw_text_screen();
  PBKitPlusPlus::NV2AState::FinishDraw();

  RunAllTests();

  LogMsg("Log file: %s\n\n", kLogPath.c_str());
  LogMsg("\n[Start / B / RShoulder] Exit\n");

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
    RenderLogScreen();

    PBKitPlusPlus::NV2AState::FinishDraw();
  }

  pb_kill();
  return 0;
}
