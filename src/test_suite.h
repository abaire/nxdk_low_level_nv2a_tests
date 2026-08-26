#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_SUITE_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_SUITE_H

struct TestCase {
  const char* name;
  bool (*test_function)();

  template <typename T>
  static constexpr TestCase From() {
    return TestCase{T::Name(), &T::Test};
  }
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_SUITE_H
