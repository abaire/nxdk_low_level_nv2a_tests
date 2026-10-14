#include "test_log.h"

#include <hal/debug.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstdarg>
#include <fstream>

#include "configure.h"
#include "printf/printf.h"

namespace {

constexpr size_t kMaxOnScreenLogLines = 18;
constexpr size_t kMaxMessageLength = 512;

enum class Phase { kNoSuite, kSuiteEvents, kTests, kInTest, kDone };

struct State {
  bool initialized{false};
  std::string path;
  Phase phase{Phase::kNoSuite};
  bool has_events{false};
  bool has_tests{false};
  //! One entry per open "events" array (suite/test/scope); true once the array
  //! has at least one item.
  std::vector<bool> array_has_item;
  DWORD suite_start_ms{0};
  DWORD test_start_ms{0};
  std::deque<std::string> on_screen_log;
};

State g_state;

std::string Indent(size_t n) { return std::string(n, ' '); }

void AppendToFile(const std::string& text) {
  if (!g_state.initialized) {
    return;
  }
  std::ofstream file(g_state.path, std::ios_base::app);
  if (!file) {
    DbgPrint("Failed to open log file %s for append\n", g_state.path.c_str());
    return;
  }
  file << text;
}

void AppendJsonString(std::string& out, const std::string& value) {
  out.push_back('"');
  for (unsigned char c : value) {
    switch (c) {
      case '"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        if (c < 0x20 || c > 0x7E) {
          char buf[8];
          snprintf_(buf, sizeof(buf), "\\u%04x", c);
          out += buf;
        } else {
          out.push_back(static_cast<char>(c));
        }
    }
  }
  out.push_back('"');
}

//! Strips the build machine's directory prefix, leaving a path relative to the
//! repository's `src` directory (e.g. `ptimer_tests/test_foo.cpp`).
std::string NormalizeSourcePath(const char* path) {
  std::string result(path ? path : "");
  for (auto& c : result) {
    if (c == '\\') {
      c = '/';
    }
  }
  auto pos = result.rfind("/src/");
  if (pos != std::string::npos) {
    result.erase(0, pos + 5);
  }
  return result;
}

std::string StripTrailingNewlines(std::string s) {
  while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) {
    s.pop_back();
  }
  return s;
}

std::string StripLeadingSpaces(const std::string& s) {
  auto start = s.find_first_not_of(' ');
  return start == std::string::npos ? std::string() : s.substr(start);
}

void AddScreenLine(const std::string& line) {
  if (line.empty()) {
    return;
  }
  g_state.on_screen_log.push_back(line);
  if (g_state.on_screen_log.size() > kMaxOnScreenLogLines) {
    g_state.on_screen_log.pop_front();
  }
}

void EchoLine(const std::string& line) {
  DbgPrint("%s\n", line.c_str());
  AddScreenLine(line);
}

DWORD ElapsedMs() { return GetTickCount() - g_state.suite_start_ms; }

//! Returns the indentation of items in the innermost open events array.
size_t EventIndent() {
  size_t depth =
      g_state.array_has_item.empty() ? 0 : g_state.array_has_item.size() - 1;
  size_t base = (g_state.phase == Phase::kInTest) ? 8 : 4;
  return base + 4 * depth;
}

//! Writes the separator and indentation that begin a new item in the
//! innermost events array.
std::string BeginEventItem() {
  std::string out;
  auto has_item = g_state.array_has_item.back();
  out += has_item ? ",\n" : "\n";
  g_state.array_has_item.back() = true;
  out += Indent(EventIndent());
  return out;
}

//! True if an event can currently be recorded in the structured log.
bool CanRecordEvent() {
  return g_state.initialized && !g_state.array_has_item.empty() &&
         (g_state.phase == Phase::kSuiteEvents ||
          g_state.phase == Phase::kInTest);
}

const char* LevelName(TestLog::Level level) {
  switch (level) {
    case TestLog::Level::kPass:
      return "pass";
    case TestLog::Level::kFail:
      return "fail";
    case TestLog::Level::kInfo:
      return "info";
    case TestLog::Level::kError:
      return "error";
  }
  return "info";
}

const char* ScreenPrefix(TestLog::Level level) {
  switch (level) {
    case TestLog::Level::kPass:
      return "  [PASS] ";
    case TestLog::Level::kFail:
      return "  [FAIL] ";
    case TestLog::Level::kError:
      return "  [ERROR] ";
    case TestLog::Level::kInfo:
      break;
  }
  return "  ";
}

void AppendLocation(std::string& out, const char* file, int line) {
  char buf[32];
  out += ", \"file\": ";
  AppendJsonString(out, NormalizeSourcePath(file));
  snprintf_(buf, sizeof(buf), ", \"line\": %d", line);
  out += buf;
}

void AppendTime(std::string& out) {
  char buf[48];
  snprintf_(buf, sizeof(buf), ", \"t_ms\": %lu",
            static_cast<unsigned long>(ElapsedMs()));
  out += buf;
}

}  // namespace

void LogValue::AppendJson(std::string& out) const {
  char buf[64];
  switch (type_) {
    case Type::kInt:
      snprintf_(buf, sizeof(buf), "%lld", static_cast<long long>(int_));
      out += buf;
      break;
    case Type::kUint:
      snprintf_(buf, sizeof(buf), "%llu",
                static_cast<unsigned long long>(uint_));
      out += buf;
      break;
    case Type::kDouble:
      if (double_ != double_ || double_ > 1e300 || double_ < -1e300) {
        out += "null";
      } else {
        snprintf_(buf, sizeof(buf), "%.6f", double_);
        out += buf;
      }
      break;
    case Type::kBool:
      out += bool_ ? "true" : "false";
      break;
    case Type::kString:
      AppendJsonString(out, string_);
      break;
  }
}

LogValue Hex32(uint32_t value) {
  char buf[16];
  snprintf_(buf, sizeof(buf), "0x%08X", static_cast<unsigned int>(value));
  return LogValue(std::string(buf));
}

void LogFields::AppendJson(std::string& out) const {
  out.push_back('{');
  bool first = true;
  for (const auto& entry : fields_) {
    if (!first) {
      out += ", ";
    }
    first = false;
    AppendJsonString(out, entry.first);
    out += ": ";
    entry.second.AppendJson(out);
  }
  out.push_back('}');
}

namespace TestLog {

bool Initialize(const std::string& path, bool truncate) {
  g_state.path = path;
  DbgPrint("Opening log file at %s\n", path.c_str());
  std::ofstream file(path,
                     truncate ? std::ios_base::trunc : std::ios_base::app);
  if (!file) {
    DbgPrint("Failed to open log file %s for output\n", path.c_str());
    g_state.initialized = false;
    return false;
  }
  g_state.initialized = true;
  return true;
}

bool IsInitialized() { return g_state.initialized; }

void BeginSuite(const std::string& suite_name) {
  g_state.suite_start_ms = GetTickCount();
  g_state.phase = Phase::kSuiteEvents;
  g_state.has_tests = false;
  g_state.array_has_item.assign(1, false);

  std::string out = "{\n  \"schema_version\": 1";
#ifdef LOCATION_URL_BASE
  out += ",\n  \"location_url_base\": ";
  AppendJsonString(out, LOCATION_URL_BASE);
#endif
  out += ",\n  \"suite\": ";
  AppendJsonString(out, suite_name);
  out += ",\n  \"events\": [";
  AppendToFile(out);
}

void EndSuite(size_t passed, size_t failed) {
  if (g_state.phase == Phase::kInTest) {
    EndTest(false);
  }

  std::string out;
  if (g_state.phase == Phase::kSuiteEvents) {
    out += "\n  ],\n  \"tests\": []";
  } else {
    out += "\n  ]";
  }

  char buf[96];
  snprintf_(buf, sizeof(buf),
            ",\n  \"summary\": {\"total\": %u, \"passed\": %u, \"failed\": %u}",
            static_cast<unsigned>(passed + failed),
            static_cast<unsigned>(passed), static_cast<unsigned>(failed));
  out += buf;
  snprintf_(buf, sizeof(buf), ",\n  \"duration_ms\": %lu\n}\n",
            static_cast<unsigned long>(ElapsedMs()));
  out += buf;
  AppendToFile(out);

  g_state.phase = Phase::kDone;
  g_state.array_has_item.clear();
}

void BeginTest(const char* name, const char* source) {
  Print("============================================================");
  Print("Test: %s", name);

  if (!g_state.initialized || (g_state.phase != Phase::kSuiteEvents &&
                               g_state.phase != Phase::kTests)) {
    return;
  }

  std::string out;
  if (g_state.phase == Phase::kSuiteEvents) {
    out += "\n  ],\n  \"tests\": [";
  }
  out += g_state.has_tests ? ",\n" : "\n";
  g_state.has_tests = true;

  char buf[48];
  out += "    {\n      \"name\": ";
  AppendJsonString(out, name);
  out += ",\n      \"source\": ";
  AppendJsonString(out, NormalizeSourcePath(source));
  g_state.test_start_ms = GetTickCount();
  snprintf_(buf, sizeof(buf), ",\n      \"started_ms\": %lu",
            static_cast<unsigned long>(ElapsedMs()));
  out += buf;
  out += ",\n      \"events\": [";
  AppendToFile(out);

  g_state.phase = Phase::kInTest;
  g_state.array_has_item.assign(1, false);
}

void EndTest(bool passed) {
  if (g_state.phase != Phase::kInTest) {
    return;
  }
  while (g_state.array_has_item.size() > 1) {
    EndScope();
  }

  char buf[96];
  std::string out = "\n      ],\n      \"result\": ";
  out += passed ? "\"pass\"" : "\"fail\"";
  snprintf_(buf, sizeof(buf), ",\n      \"duration_ms\": %lu\n    }",
            static_cast<unsigned long>(GetTickCount() - g_state.test_start_ms));
  out += buf;
  AppendToFile(out);

  g_state.phase = Phase::kTests;
  g_state.array_has_item.clear();
}

void Event(Level level, const char* file, int line, const LogFields& fields,
           const char* fmt, ...) {
  char buf[kMaxMessageLength];
  va_list args;
  va_start(args, fmt);
  vsnprintf_(buf, sizeof(buf), fmt, args);
  va_end(args);

  std::string message = StripTrailingNewlines(buf);
  EchoLine(ScreenPrefix(level) + message);

  if (!CanRecordEvent()) {
    return;
  }

  std::string out = BeginEventItem();
  out += "{\"level\": \"";
  out += LevelName(level);
  out += "\", \"message\": ";
  AppendJsonString(out, StripLeadingSpaces(message));
  AppendLocation(out, file, line);
  AppendTime(out);
  if (!fields.empty()) {
    out += ", \"fields\": ";
    fields.AppendJson(out);
  }
  out.push_back('}');
  AppendToFile(out);
}

void HexDump(const char* file, int line, const char* label, const void* data,
             size_t len) {
  const auto* p = static_cast<const uint8_t*>(data);
  char buf[kMaxMessageLength];
  snprintf_(buf, sizeof(buf), "%s (%u bytes):", label,
            static_cast<unsigned>(len));
  EchoLine(std::string("  ") + buf);

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
    snprintf_(buf, sizeof(buf), "    %04X:  %s |%s|", static_cast<unsigned>(i),
              hex, ascii);
    EchoLine(buf);
  }

  if (!CanRecordEvent()) {
    return;
  }

  std::string out = BeginEventItem();
  out += "{\"level\": \"dump\", \"label\": ";
  AppendJsonString(out, label);
  snprintf_(buf, sizeof(buf), ", \"size\": %u", static_cast<unsigned>(len));
  out += buf;
  out += ", \"bytes\": \"";
  static const char kHexDigits[] = "0123456789ABCDEF";
  for (size_t i = 0; i < len; ++i) {
    out.push_back(kHexDigits[p[i] >> 4]);
    out.push_back(kHexDigits[p[i] & 0xF]);
  }
  out.push_back('"');
  AppendLocation(out, file, line);
  AppendTime(out);
  out.push_back('}');
  AppendToFile(out);
}

void BeginScope(const char* file, int line, const char* name) {
  EchoLine(std::string("  ") + name + ":");

  if (!CanRecordEvent()) {
    return;
  }

  std::string out = BeginEventItem();
  size_t indent = EventIndent();
  out += "{\n" + Indent(indent + 2) + "\"scope\": ";
  AppendJsonString(out, name);
  AppendLocation(out, file, line);
  AppendTime(out);
  out += ",\n" + Indent(indent + 2) + "\"events\": [";
  AppendToFile(out);

  g_state.array_has_item.push_back(false);
}

void EndScope() {
  if (g_state.array_has_item.size() < 2) {
    return;
  }
  g_state.array_has_item.pop_back();
  size_t indent = EventIndent();
  AppendToFile("\n" + Indent(indent + 2) + "]\n" + Indent(indent) + "}");
}

void Print(const char* fmt, ...) {
  char buf[kMaxMessageLength];
  va_list args;
  va_start(args, fmt);
  vsnprintf_(buf, sizeof(buf), fmt, args);
  va_end(args);
  EchoLine(StripTrailingNewlines(buf));
}

const std::deque<std::string>& GetOnScreenLog() {
  return g_state.on_screen_log;
}

void ClearOnScreenLog() { g_state.on_screen_log.clear(); }

}  // namespace TestLog
