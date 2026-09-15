#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_MAX_LINE_COUNT_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_MAX_LINE_COUNT_H

//! Validates that the M2MF engine correctly executes transfers with the maximum
//! allowable hardware line count of 2047 lines in a single kick.
//!
//! Validates that:
//! 1. A transfer of 2047 lines copies all lines accurately without data
//! truncation.
//! 2. Pattern verification matches across all 2047 rows.
//! 3. Destination pre-guard and post-guard canaries remain completely
//! unmodified.
struct TestM2MFMaxLineCount {
  static constexpr const char* Name() { return "M2MF Max Line Count (2047)"; }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_MAX_LINE_COUNT_H
