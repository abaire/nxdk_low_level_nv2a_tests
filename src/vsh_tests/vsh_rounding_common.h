#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_VSH_ROUNDING_COMMON_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_VSH_ROUNDING_COMMON_H

#include <cstddef>
#include <cstdint>

#include "vsh_test_common.h"

//! Maximum number of rounding models recorded for a single probe.
constexpr size_t kMaxRoundingModels = 5;

//! The result predicted for a probe by a particular rounding model.
//!
//! Model names used by the rounding tests:
//! - "rn": the exact result rounded once to nearest, ties to even.
//! - "rz": the exact result truncated once toward zero.
//! - "rn_rn" / "rz_rz": MAD with the product and then the sum each rounded to
//! nearest / truncated.
//! - "tp_rz": dot product with each product truncated, the products summed
//! exactly, and the sum truncated.
//! - "seq_rz": dot product with each product and each partial sum truncated,
//! accumulating x, y, z, w in order.
//! - "win_rz": the observed nv2a adder (ADD, MAD's addition and the DP3, DPH
//! and DP4 accumulator). The addends (truncated products for MAD and dot
//! products) are aligned to the largest one and each is truncated to a
//! multiple of 2^-28 relative to the largest addend's leading bit, i.e. a
//! 29-bit window with no sticky bit. The aligned addends are summed exactly and
//! the sum is truncated toward zero.
struct RoundingModel {
  //! The model name, or nullptr for an unused entry.
  const char* name;
  uint32_t bits;
};

//! Reference results for a single rounding probe.
struct RoundingExpectation {
  //! Whether `expected` is checked. Unchecked probes are recorded only, for
  //! behavior that has not yet been captured from hardware.
  bool checked;
  uint32_t expected;
  RoundingModel models[kMaxRoundingModels];
};

//! Logs `actual` along with every model in `expectation` and the names of the
//! models it matches. Checked expectations log a pass or fail; unchecked ones
//! log an info event. Returns false only if a checked expectation failed.
bool VshReportRounding(const char* op, const char* label, uint32_t actual,
                       const RoundingExpectation& expectation);

//! A probe of a two-operand MAC instruction.
struct BinaryRoundingCase {
  const char* label;
  uint32_t a;
  uint32_t b;
  RoundingExpectation expectation;
};

//! Runs `cases` through a shader computing c[188 + i] = OP(c[96 + 2i],
//! c[97 + 2i]) per component for i in [0, 3]. Case k of each draw of 16 uses
//! component k % 4 of vector k / 4.
bool VshRunBinaryRoundingCases(ComputeShader shader, const char* op,
                               const BinaryRoundingCase* cases, size_t count);

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_VSH_ROUNDING_COMMON_H
