#include "m2mf_test_common.h"

#include <hal/debug.h>
#include <pbkit/nv_objects.h>
#include <pbkit/pbkit.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstdarg>
#include <cstring>
#include <deque>
#include <string>

#include "logger.h"
#include "printf/printf.h"

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

bool M2MFInit() {
  PBKitPlusPlus::Pushbuffer::Initialize();
  if (PBKitPlusPlus::NV2AState::InitializeM2M() < 0) {
    LogMsg("  [ERROR] gpum_init failed!\n");
    return false;
  }
  return true;
}

void M2MFTeardown() { PBKitPlusPlus::NV2AState::PBKitBusyWait(); }

uint32_t M2MFGetPhysicalAddress(const void* p) {
  uintptr_t va = reinterpret_cast<uintptr_t>(p);
  if (va >= 0x80000000 && va < 0xC0000000) {
    return static_cast<uint32_t>(va & 0x1FFFFFFF);
  }
  return static_cast<uint32_t>(
      reinterpret_cast<uintptr_t>(MmGetPhysicalAddress(const_cast<PVOID>(p))));
}

void M2MFFillPattern(void* buf, size_t size, uint8_t seed) {
  uint8_t* b = static_cast<uint8_t*>(buf);
  uint8_t val = seed;
  for (size_t i = 0; i < size; ++i) {
    b[i] = val;
    val = static_cast<uint8_t>((val * 109 + 89) ^ (i & 0xFF));
  }
}

bool M2MFVerifyPattern(const void* buf, size_t size, uint8_t seed) {
  const uint8_t* b = static_cast<const uint8_t*>(buf);
  uint8_t val = seed;
  for (size_t i = 0; i < size; ++i) {
    if (b[i] != val) {
      return false;
    }
    val = static_cast<uint8_t>((val * 109 + 89) ^ (i & 0xFF));
  }
  return true;
}

bool M2MFVerifyCanaries(const void* buf, size_t size, uint8_t canary) {
  const uint8_t* b = static_cast<const uint8_t*>(buf);
  for (size_t i = 0; i < size; ++i) {
    if (b[i] != canary) {
      return false;
    }
  }
  return true;
}

void LogHexDump(const char* label, const void* data, size_t len) {
  const auto* p = static_cast<const uint8_t*>(data);
  LogMsg("  %s (%u bytes):\n", label, static_cast<uint32_t>(len));
  for (size_t i = 0; i < len; i += 16) {
    char hex[50] = {0};
    char ascii[17] = {0};
    size_t chunk = (len - i < 16) ? (len - i) : 16;
    size_t hex_pos = 0;
    for (size_t j = 0; j < 16; ++j) {
      if (j < chunk) {
        uint8_t byte = p[i + j];
        hex_pos +=
            snprintf_(hex + hex_pos, sizeof(hex) - hex_pos, "%02X ", byte);
        ascii[j] = (byte >= 32 && byte <= 126) ? static_cast<char>(byte) : '.';
      } else {
        hex_pos += snprintf_(hex + hex_pos, sizeof(hex) - hex_pos, "   ");
        ascii[j] = ' ';
      }
    }
    ascii[chunk] = '\0';
    LogMsg("    %04X:  %s |%s|\n", static_cast<uint32_t>(i), hex, ascii);
  }
}
