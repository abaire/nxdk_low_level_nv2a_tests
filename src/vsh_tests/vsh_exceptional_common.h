#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_VSH_EXCEPTIONAL_COMMON_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_VSH_EXCEPTIONAL_COMMON_H

#include <cstddef>
#include <cstdint>

#include "vsh_rounding_common.h"

//! Raw float32 bit patterns used as operands by the exceptional value tests.
//! Operands are always carried as bits so that NaN payloads are never touched
//! by CPU float arithmetic.
namespace VshBits {
constexpr uint32_t kPosZero = 0x00000000;
constexpr uint32_t kNegZero = 0x80000000;
constexpr uint32_t kPosInf = 0x7F800000;
constexpr uint32_t kNegInf = 0xFF800000;
//! The NaN produced by nv2a DP4 and DPH (observed on hardware).
constexpr uint32_t kNaN = 0x7FFFFFFF;
//! The IEEE 754 default quiet NaN.
constexpr uint32_t kQNaN = 0x7FC00000;
constexpr uint32_t kNegNaN = 0xFFC00000;
//! A signaling NaN with a non-canonical payload.
constexpr uint32_t kPayloadNaN = 0x7FA5A5A5;
constexpr uint32_t kFltMax = 0x7F7FFFFF;
constexpr uint32_t kNegFltMax = 0xFF7FFFFF;
//! 2^-126
constexpr uint32_t kMinNormal = 0x00800000;
//! 2^-127
constexpr uint32_t kDenormal = 0x00400000;
constexpr uint32_t kNegDenormal = 0x80400000;
constexpr uint32_t kOne = 0x3F800000;
constexpr uint32_t kNegOne = 0xBF800000;
constexpr uint32_t kOneAndHalf = 0x3FC00000;
constexpr uint32_t kNegOneAndHalf = 0xBFC00000;
constexpr uint32_t kTwo = 0x40000000;
constexpr uint32_t kNegTwo = 0xC0000000;
constexpr uint32_t kThree = 0x40400000;
constexpr uint32_t kFour = 0x40800000;
constexpr uint32_t kNegFour = 0xC0800000;
constexpr uint32_t kHalf = 0x3F000000;
constexpr uint32_t kNegHalf = 0xBF000000;
}  // namespace VshBits

//! A checked expectation recorded under the model name "hw": the result
//! observed on hardware.
constexpr RoundingExpectation Expect(uint32_t hw) {
  return {true, hw, {{"hw", hw}}};
}

//! Reports each component of `actual` via VshReportRounding, labeled
//! "<label>.<component>". `context` is logged with "case" set to `label` and
//! "component" set to the component name.
bool VshReportVector(const char* op, const char* label,
                     const uint32_t actual[4],
                     const RoundingExpectation expected[4],
                     const VshRecordContext& context);

//! A probe of MAD with the three operands of a single component.
struct TernaryCase {
  const char* label;
  uint32_t a;
  uint32_t b;
  uint32_t c;
  RoundingExpectation expectation;
};

//! Runs `cases` through a shader computing c[188 + i] = c[96 + 3i] *
//! c[97 + 3i] + c[98 + 3i] (kRoundMad). Case k of each draw of 16 uses
//! component k % 4 of vector k / 4. Results are logged in group "mac".
bool VshRunTernaryCases(ComputeShader shader, const char* op,
                        const TernaryCase* cases, size_t count);

//! A probe of DP3, DPH and DP4 with whole vectors.
struct ExceptionalDotCase {
  const char* label;
  uint32_t a[4];
  uint32_t b[4];
  RoundingExpectation dp3;
  RoundingExpectation dph;
  RoundingExpectation dp4;
};

//! Runs `cases` through kRoundDot, 4 cases per draw. Results are logged in
//! group "mac".
bool VshRunExceptionalDotCases(const ExceptionalDotCase* cases, size_t count);

//! A probe of an ILU operation on a single scalar input whose result is
//! replicated to every component.
struct ScalarIluCase {
  const char* label;
  uint32_t input;
  RoundingExpectation expectation;
};

//! Runs `cases` through a shader computing c[188 + i] = OP(component i of
//! c[96]), checking the x component of each result. 4 cases per draw. Results
//! are logged in group "ilu".
bool VshRunScalarIluCases(ComputeShader shader, const char* op,
                          const ScalarIluCase* cases, size_t count);

//! A probe of an ILU operation on a single scalar input producing a vector.
struct VectorIluCase {
  const char* label;
  uint32_t input;
  RoundingExpectation expectation[4];
};

//! As VshRunScalarIluCases, checking every component of each result.
bool VshRunVectorIluCases(ComputeShader shader, const char* op,
                          const VectorIluCase* cases, size_t count);

//! A probe of a vector operation with up to two vector operands.
struct VectorCase {
  const char* label;
  uint32_t a[4];
  uint32_t b[4];
  RoundingExpectation expectation[4];
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_VSH_EXCEPTIONAL_COMMON_H
