#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_RUNNER_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_RUNNER_H

#include <string>
#include <vector>

#include "test_suite.h"

//! Harness for configuring, running, and reporting results of test suites.
//! Supports reading a configuration file (`c:\test_suite_config.ini`) to
//! override the log output path and optionally force an SMBus system shutdown
//! for headless automated testing.
class TestRunner {
 public:
  TestRunner(const std::string& suite_name, const std::string& default_log_path,
             const TestCase* tests, size_t num_tests);

  int Run();

 private:
  void LoadConfiguration();
  bool SetupLogging();
  void RenderLogScreen();
  void RunAllTests();
  void ShutdownSystem();

  std::string suite_name_;
  std::string log_path_;
  bool shutdown_on_completion_ = false;
  const TestCase* tests_;
  size_t num_tests_;
  std::vector<bool> results_;
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_RUNNER_H
