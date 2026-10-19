#include "test_vsh_ilu_rcp.h"

#include <iterator>

#include "vsh_test_common.h"

namespace {

constexpr uint32_t kPosInf = 0x7F800000;
constexpr uint32_t kNegInf = 0xFF800000;
constexpr uint32_t kPosNaN = 0x7FC00000;
constexpr uint32_t kNegNaN = 0xFFC00000;
constexpr uint32_t kPosMax = 0x7F7FFFFF;
constexpr uint32_t kNegMax = 0xFF7FFFFF;
constexpr uint32_t kPosMin = 0x00800000;
constexpr uint32_t kNegMin = 0x80800000;
constexpr uint32_t kNegMaxSubnormal = 0x807FFFFF;
constexpr uint32_t kPosMinSubnormal = 0x00000001;
constexpr uint32_t kNegMinSubnormal = 0x80000001;

constexpr uint32_t kPos2Pow126 = 0x7E800000;
constexpr uint32_t kNeg2Pow126 = 0xFE800000;
constexpr uint32_t kPosRcp2123 = 0x3EF12B0D;
constexpr uint32_t kNegRcp2123 = 0xBEF12B0D;
constexpr uint32_t kPosZero = 0x00000000;
constexpr uint32_t kNegZero = 0x80000000;
constexpr uint32_t kCanonicalNaN = 0x7FFFFFFF;

}  // namespace

bool TestVshIluRcp::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  const VshConstant inputs[] = {
      {96, {FloatBits(1.0f), FloatBits(2.123f), kPosMin, kPosMax}},
      {97, {FloatBits(-1.0f), FloatBits(-2.123f), kNegMin, kNegMax}},
      {98, {0, kPosInf, kNegInf, kPosNaN}},
      {99, {kNegNaN, kNegMaxSubnormal, kPosMinSubnormal, kNegMinSubnormal}},
  };

  uint32_t outputs[4][4];
  if (!VshCompute(ComputeShader::kIluRcp, inputs, std::size(inputs), 188, 4,
                  outputs)) {
    TEST_FAIL("Compute draw failed.");
    return false;
  }

  using E = ExpectedComponent;
  const E expected[4][4] = {
      {E::Float(1.0f), E::Bits(kPosRcp2123), E::Bits(kPos2Pow126),
       E::Bits(kPosZero)},
      {E::Float(-1.0f), E::Bits(kNegRcp2123), E::Bits(kNeg2Pow126),
       E::Bits(kNegZero)},
      {E::Bits(kPosInf), E::Bits(kPosZero), E::Bits(kNegZero),
       E::Bits(kCanonicalNaN)},
      {E::Bits(kCanonicalNaN), E::Bits(kNegInf), E::Bits(kPosInf),
       E::Bits(kNegInf)},
  };
  static constexpr const char* kLabels[] = {
      "RCP(1, 2.123, Min, Max)",
      "RCP(-1, -2.123, -Min, -Max)",
      "RCP(0, Inf, -Inf, NaN)",
      "RCP(-NaN, -MaxSub, MinSub, -MinSub)",
  };

  bool pass = true;
  for (size_t i = 0; i < std::size(kLabels); ++i) {
    pass &= VshCheckVector(kLabels[i], outputs[i], expected[i]);
  }
  return pass;
}
