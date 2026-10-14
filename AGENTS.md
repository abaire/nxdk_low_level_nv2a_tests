# Guidelines for AI Coding Agents

This repository contains low-level hardware validation tests for the Xbox NV2A GPU.
When contributing to or modifying this codebase, the following practices are **mandatory**:

## 1. Documentation Requirements
- **Doxygen-style Doc Comments**: Every test class header must include Doxygen-style doc comments (`//!`) documenting:
  - High-level purpose of the test.
  - An explicit enumerated list of behaviors/invariants validated by the test.
- **Concise Implementation Comments**:
  - Do not add redundant or obvious comments that merely restate what code does.
  - Do not use decorative block banners or divide test methods into artificial "Part X" sections; if a test is validating distinct behaviors, separate them into distinct `TestCase` structs.

## 2. Test Logging
- Tests report results with the macros in `src/test_log.h` (`TEST_PASS`, `TEST_FAIL`, `TEST_INFO`, `TEST_ERROR`, their `_EX` variants taking `TestFields("key", value, ...)`, `TEST_SCOPE`, `TEST_HEXDUMP`). Do not print/log directly; the macros echo to the screen and record structured JSON with the source file and line.
- Do not print banners or test names; `TestRunner` opens/closes each test in the log. Every test class must provide `Name()` and `Source()` (`return __FILE__;`).

## 3. Code Formatting Requirements
- **Run `clang-format`**: Before completing any task, format all newly created or modified source and header files (`*.cpp`, `*.h`, `*.c`) using `clang-format`:
  ```bash
  clang-format -i <path/to/changed/file>
  ```
- Ensure changes adhere cleanly to repository indentation (2 spaces) and style.

## 4. Timing and Hardware Verification Considerations
- Avoid I/O (such as `LogMsg`, `DbgPrint`, or filesystem writes) between time-sensitive MMIO register operations, as disk I/O on the Xbox FATX filesystem can take several milliseconds and introduce race conditions (e.g. Setting alarms in the past).
