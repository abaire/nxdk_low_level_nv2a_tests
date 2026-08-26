#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_LOGGER_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_LOGGER_H

#include <fstream>
#include <string>

class Logger {
 public:
  static void Initialize(const std::string& log_path, bool truncate_log);
  static bool IsInitialized() { return singleton_ != nullptr; }
  static std::ofstream Log();

 private:
  explicit Logger(const std::string& path, bool truncate_log = false);

  std::string log_path_;

  static Logger* singleton_;
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_LOGGER_H
