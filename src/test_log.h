#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_LOG_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_LOG_H

#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

//! Structured test logging.
//!
//! Events are echoed as plain text to the debug console and the on-screen log
//! and, if TestLog::Initialize has been called, appended to a nested JSON
//! document:
//!
//!   suite
//!     events[]            (events emitted before the first test)
//!     tests[]
//!       events[]          (pass/fail/info/error/dump events and scopes)
//!       result, duration_ms
//!     summary
//!
//! Every write opens the file in append mode and closes it again so that the
//! file reflects progress up to the point of a crash. A crash leaves the
//! document truncated (and therefore invalid JSON); the innermost unclosed
//! test/event identifies where the failure occurred.
//!
//! Tests should use the TEST_* macros below, which capture the source location.

//! A single typed value attached to an event.
class LogValue {
 public:
  enum class Type { kInt, kUint, kDouble, kBool, kString };

  template <typename T,
            typename std::enable_if<std::is_integral<T>::value &&
                                        !std::is_same<T, bool>::value &&
                                        std::is_signed<T>::value,
                                    int>::type = 0>
  LogValue(T v) : type_(Type::kInt), int_(v) {}

  template <typename T,
            typename std::enable_if<std::is_integral<T>::value &&
                                        !std::is_same<T, bool>::value &&
                                        !std::is_signed<T>::value,
                                    int>::type = 0>
  LogValue(T v) : type_(Type::kUint), uint_(v) {}

  LogValue(bool v) : type_(Type::kBool), bool_(v) {}
  LogValue(double v) : type_(Type::kDouble), double_(v) {}
  LogValue(const char* v) : type_(Type::kString), string_(v ? v : "") {}
  LogValue(std::string v) : type_(Type::kString), string_(std::move(v)) {}

  //! Appends the JSON representation of this value to `out`.
  void AppendJson(std::string& out) const;

 private:
  Type type_;
  int64_t int_{0};
  uint64_t uint_{0};
  double double_{0.0};
  bool bool_{false};
  std::string string_;
};

//! Returns `value` formatted as a "0x%08X" string value.
LogValue Hex32(uint32_t value);

//! Ordered set of key/value pairs attached to an event.
class LogFields {
 public:
  LogFields() = default;

  LogFields& Add(const char* key, LogValue value) {
    fields_.emplace_back(key, std::move(value));
    return *this;
  }

  [[nodiscard]] bool empty() const { return fields_.empty(); }
  void AppendJson(std::string& out) const;

 private:
  std::vector<std::pair<std::string, LogValue>> fields_;
};

namespace test_log_internal {
inline void AddFields(LogFields&) {}

template <typename V, typename... Rest>
void AddFields(LogFields& fields, const char* key, V&& value, Rest&&... rest) {
  fields.Add(key, LogValue(std::forward<V>(value)));
  AddFields(fields, std::forward<Rest>(rest)...);
}
}  // namespace test_log_internal

//! Builds a LogFields from alternating key, value arguments:
//!   TestFields("expected", a, "actual", b)
template <typename... Args>
LogFields TestFields(Args&&... args) {
  static_assert(sizeof...(Args) % 2 == 0, "TestFields takes key/value pairs");
  LogFields fields;
  test_log_internal::AddFields(fields, std::forward<Args>(args)...);
  return fields;
}

namespace TestLog {

enum class Level { kPass, kFail, kInfo, kError };

//! Opens `path` for structured output (optionally truncating) and returns true
//! if the file could be opened.
bool Initialize(const std::string& path, bool truncate);
bool IsInitialized();

void BeginSuite(const std::string& suite_name);
void EndSuite(size_t passed, size_t failed);

//! Starts a test. `source` is the file declaring the test class.
void BeginTest(const char* name, const char* source);
void EndTest(bool passed);

//! Logs an event within the current test (or the suite, before any test has
//! started). The formatted message is also echoed to the console and screen.
void Event(Level level, const char* file, int line, const LogFields& fields,
           const char* fmt, ...) __attribute__((format(printf, 5, 6)));

//! Logs a hex dump of `data`. Echoed to the screen as formatted text, recorded
//! in the structured log as a single event holding the bytes as hex.
void HexDump(const char* file, int line, const char* label, const void* data,
             size_t len);

void BeginScope(const char* file, int line, const char* name);
void EndScope();

//! Echoes text to the console and on-screen log only (not the structured log).
void Print(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

const std::deque<std::string>& GetOnScreenLog();
void ClearOnScreenLog();

//! RAII helper for TEST_SCOPE.
class ScopeGuard {
 public:
  ScopeGuard(const char* file, int line, const char* name) {
    BeginScope(file, line, name);
  }
  ~ScopeGuard() { EndScope(); }
  ScopeGuard(const ScopeGuard&) = delete;
  ScopeGuard& operator=(const ScopeGuard&) = delete;
};

}  // namespace TestLog

#define TEST_LOG_CONCAT_INNER(a, b) a##b
#define TEST_LOG_CONCAT(a, b) TEST_LOG_CONCAT_INNER(a, b)

#define TEST_LOG_EVENT(level, ...) \
  TestLog::Event(level, __FILE__, __LINE__, LogFields(), __VA_ARGS__)
#define TEST_LOG_EVENT_EX(level, fields, ...) \
  TestLog::Event(level, __FILE__, __LINE__, fields, __VA_ARGS__)

//! printf-style events. The _EX variants take a TestFields(...) argument first.
#define TEST_PASS(...) TEST_LOG_EVENT(TestLog::Level::kPass, __VA_ARGS__)
#define TEST_FAIL(...) TEST_LOG_EVENT(TestLog::Level::kFail, __VA_ARGS__)
#define TEST_INFO(...) TEST_LOG_EVENT(TestLog::Level::kInfo, __VA_ARGS__)
#define TEST_ERROR(...) TEST_LOG_EVENT(TestLog::Level::kError, __VA_ARGS__)

#define TEST_PASS_EX(fields, ...) \
  TEST_LOG_EVENT_EX(TestLog::Level::kPass, fields, __VA_ARGS__)
#define TEST_FAIL_EX(fields, ...) \
  TEST_LOG_EVENT_EX(TestLog::Level::kFail, fields, __VA_ARGS__)
#define TEST_INFO_EX(fields, ...) \
  TEST_LOG_EVENT_EX(TestLog::Level::kInfo, fields, __VA_ARGS__)
#define TEST_ERROR_EX(fields, ...) \
  TEST_LOG_EVENT_EX(TestLog::Level::kError, fields, __VA_ARGS__)

//! Nests all events logged until the end of the enclosing C++ scope.
#define TEST_SCOPE(name)                                          \
  TestLog::ScopeGuard TEST_LOG_CONCAT(test_log_scope_, __LINE__)( \
      __FILE__, __LINE__, name)

#define TEST_HEXDUMP(label, data, len) \
  TestLog::HexDump(__FILE__, __LINE__, label, data, len)

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_LOG_H
