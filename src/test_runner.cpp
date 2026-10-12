#include "test_runner.h"

#include <SDL.h>
#include <hal/debug.h>
#include <hal/video.h>
#include <pbkit/pbkit.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <sstream>

#include "logger.h"
#include "nv2astate.h"
#include "test_util.h"

static const std::string kConfigFilePath = R"(c:\test_suite_config.ini)";

static inline std::string Trim(const std::string& str) {
  auto start = str.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return "";
  }
  auto end = str.find_last_not_of(" \t\r\n");
  return str.substr(start, end - start + 1);
}

static inline std::string ToLower(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return str;
}

TestRunner::TestRunner(const std::string& suite_name,
                       const std::string& default_log_path,
                       const TestCase* tests, size_t num_tests)
    : suite_name_(suite_name),
      log_path_(default_log_path),
      tests_(tests),
      num_tests_(num_tests) {}

void TestRunner::LoadConfiguration() {
  debugPrint("Mounting Drive C: to check for configuration...\n");
  if (!EnsureDriveMounted('C', false)) {
    DbgPrint("WARNING: Failed to mount Drive C:! Cannot check for %s\n",
             kConfigFilePath.c_str());
    return;
  }

  std::ifstream config_file(kConfigFilePath);
  if (!config_file.is_open()) {
    DbgPrint("No configuration file found at %s. Using default settings.\n",
             kConfigFilePath.c_str());
    return;
  }

  DbgPrint("Loading configuration from %s...\n", kConfigFilePath.c_str());
  std::string line;
  while (std::getline(config_file, line)) {
    line = Trim(line);
    if (line.empty() || line[0] == '#' || line[0] == ';') {
      continue;
    }

    size_t delimiter_pos = line.find('=');
    if (delimiter_pos == std::string::npos) {
      continue;
    }

    std::string key = ToLower(Trim(line.substr(0, delimiter_pos)));
    std::string value = Trim(line.substr(delimiter_pos + 1));

    if (key == "log_path") {
      if (!value.empty()) {
        log_path_ = value;
        DbgPrint("Configuration override: log_path = %s\n", log_path_.c_str());
      }
    } else if (key == "shutdown") {
      std::string val_lower = ToLower(value);
      if (val_lower == "true" || val_lower == "1" || val_lower == "yes" ||
          val_lower == "on") {
        shutdown_on_completion_ = true;
        DbgPrint("Configuration override: shutdown = true\n");
      } else {
        shutdown_on_completion_ = false;
        DbgPrint("Configuration override: shutdown = false\n");
      }
    }
  }
}

bool TestRunner::SetupLogging() {
  char drive_letter = 'E';
  if (log_path_.size() >= 2 && log_path_[1] == ':') {
    drive_letter = static_cast<char>(std::toupper(log_path_[0]));
  }

  debugPrint("Mounting Drive %c: for log output...\n", drive_letter);
  if (EnsureDriveMounted(drive_letter, false)) {
    size_t last_slash = log_path_.find_last_of("\\/");
    if (last_slash != std::string::npos) {
      EnsureFolderExists(log_path_.substr(0, last_slash));
    }
    Logger::Initialize(log_path_, true);
    LogMsg("Mounted Drive %c: and initialized log at %s\n", drive_letter,
           log_path_.c_str());
    return true;
  }

  DbgPrint("WARNING: Failed to mount Drive %c:! Disk logging disabled.\n",
           drive_letter);
  return false;
}

void TestRunner::RenderLogScreen() {
  pb_wait_for_vbl();
  pb_reset();
  pb_target_back_buffer();
  ClearScreen(0);
  pb_print("  Xbox NV2A (%s) Validation Suite\n", suite_name_.c_str());
  pb_print("============================================================\n\n");

  const auto& log = GetOnScreenLog();
  for (const auto& line : log) {
    pb_print("%s\n", line.c_str());
  }

  pb_draw_text_screen();
  PBKitPlusPlus::NV2AState::FinishDraw();
}

void TestRunner::RunAllTests() {
  results_.clear();
  results_.reserve(num_tests_);

  for (size_t i = 0; i < num_tests_; ++i) {
    bool result = tests_[i].test_function();
    results_.push_back(result);
    RenderLogScreen();
  }

  pb_erase_text_screen();
  ClearOnScreenLog();

  for (size_t i = 0; i < num_tests_; ++i) {
    LogMsg("  %2d %-32s : %s\n", static_cast<int>(i + 1), tests_[i].name,
           results_[i] ? "PASS" : "FAIL");
  }
  LogMsg("See %s\n\n", log_path_.c_str());
}

void TestRunner::ShutdownSystem() {
  DbgPrint("Shutting down system via SMBus...\n");
  const UCHAR kSMCSlaveAddress = 0x20;
  const UCHAR kSMCRegisterPower = 0x02;
  const UCHAR kSMCPowerShutdown = 0x80;
  HalWriteSMBusValue(kSMCSlaveAddress, kSMCRegisterPower, FALSE,
                     kSMCPowerShutdown);
  for (;;) {
    Sleep(1000);
  }
}

int TestRunner::Run() {
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

  LoadConfiguration();
  SetupLogging();

  pb_show_front_screen();
  debugClearScreen();

  pb_wait_for_vbl();
  pb_reset();
  pb_target_back_buffer();
  ClearScreen(0);
  pb_print("  Xbox NV2A (%s) Validation Suite\n", suite_name_.c_str());
  pb_print("============================================================\n\n");
  pb_print("Testing (this may take some time)...\n");
  pb_draw_text_screen();
  PBKitPlusPlus::NV2AState::FinishDraw();

  LogMsg("Running test suite...\n");
  RunAllTests();

  if (shutdown_on_completion_) {
    ShutdownSystem();
  }

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
                btn == SDL_CONTROLLER_BUTTON_START) {
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

    pb_print("  Xbox NV2A (%s) Validation Suite\n", suite_name_.c_str());
    pb_print("Log file: %s\n\n", log_path_.c_str());

    const auto& log = GetOnScreenLog();
    for (const auto& line : log) {
      pb_print("%s\n", line.c_str());
    }

    pb_print("\n[Black] Exit\n");

    pb_draw_text_screen();
    PBKitPlusPlus::NV2AState::FinishDraw();
  }

  pb_kill();
  return 0;
}
