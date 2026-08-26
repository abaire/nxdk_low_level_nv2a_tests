#include "logger.h"

#include <hal/debug.h>

#include <cassert>
#include <iostream>

#include "xboxkrnl/xboxkrnl.h"

Logger* Logger::singleton_ = nullptr;

Logger::Logger(const std::string& log_path, bool truncate_log)
    : log_path_(log_path) {
  const char* p = log_path.c_str();
  DbgPrint("Opening log file at %s\n", p);

  if (truncate_log) {
    auto log_file = std::ofstream(log_path, std::ios_base::trunc);
    if (!log_file) {
      DbgPrint("Failed to open log file %s for output\n", p);
    }
  }
}

void Logger::Initialize(const std::string& log_path, bool truncate_log) {
  assert(!singleton_ && "Invalid attempt to initialize logger twice.");

  singleton_ = new Logger(log_path, truncate_log);
}

std::ofstream Logger::Log() {
  assert(singleton_ && "Attempt to use Logger before Initialize");
  auto log_file = std::ofstream(singleton_->log_path_, std::ios_base::app);
  if (!log_file) {
    DbgPrint("Failed to open log file %s for append\n",
             singleton_->log_path_.c_str());
  }
  return log_file;
}
