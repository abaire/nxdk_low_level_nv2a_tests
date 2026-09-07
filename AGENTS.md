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

## 2. Code Formatting Requirements
- **Run `clang-format`**: Before completing any task, format all newly created or modified source and header files (`*.cpp`, `*.h`, `*.c`) using `clang-format`:
  ```bash
  clang-format -i <path/to/changed/file>
  ```
- Ensure changes adhere cleanly to repository indentation (2 spaces) and style.

## 3. Timing and Hardware Verification Considerations
- Avoid I/O (such as `LogMsg`, `DbgPrint`, or filesystem writes) between time-sensitive MMIO register operations, as disk I/O on the Xbox FATX filesystem can take several milliseconds and introduce race conditions (e.g. Setting alarms in the past).
