#include "test_vsh_rounding_dot.h"

#include <algorithm>
#include <iterator>

#include "vsh_rounding_common.h"

namespace {

struct DotCase {
  const char* label;
  uint32_t a[4];
  uint32_t b[4];
  RoundingExpectation dp3;
  RoundingExpectation dph;
  RoundingExpectation dp4;
};

constexpr DotCase kCases[] = {
    {"(a, RN(a * b)) . (b, -1), +0.75 ulp",
     {0x3F800800, 0x3F801401, 0x00000000, 0x00000000},
     {0x3F800C00, 0xBF800000, 0x00000000, 0x00000000},
     {true,
      0xB4000000,
      {{"rn", 0xB3000000},
       {"rz", 0xB3000000},
       {"tp_rz", 0xB4000000},
       {"seq_rz", 0xB4000000},
       {"win_rz", 0xB4000000}}},
     {true,
      0xB4000000,
      {{"rn", 0xB3000000},
       {"rz", 0xB3000000},
       {"tp_rz", 0xB4000000},
       {"seq_rz", 0xB4000000},
       {"win_rz", 0xB4000000}}},
     {true,
      0xB4000000,
      {{"rn", 0xB3000000},
       {"rz", 0xB3000000},
       {"tp_rz", 0xB4000000},
       {"seq_rz", 0xB4000000},
       {"win_rz", 0xB4000000}}}},
    {"(a, b, c) . (1, 1, -1), +0.75 ulp",
     {0x3F800000, 0x33C00000, 0x3F800001, 0x00000000},
     {0x3F800000, 0x3F800000, 0xBF800000, 0x00000000},
     {true,
      0xB3000000,
      {{"rn", 0xB3000000},
       {"rz", 0xB3000000},
       {"tp_rz", 0xB3000000},
       {"seq_rz", 0xB4000000},
       {"win_rz", 0xB3000000}}},
     {true,
      0xB3000000,
      {{"rn", 0xB3000000},
       {"rz", 0xB3000000},
       {"tp_rz", 0xB3000000},
       {"seq_rz", 0xB4000000},
       {"win_rz", 0xB3000000}}},
     {true,
      0xB3000000,
      {{"rn", 0xB3000000},
       {"rz", 0xB3000000},
       {"tp_rz", 0xB3000000},
       {"seq_rz", 0xB4000000},
       {"win_rz", 0xB3000000}}}},
    {"(a, b) . (1, 1), +0.75 ulp",
     {0x3F800000, 0x33C00000, 0x00000000, 0x00000000},
     {0x3F800000, 0x3F800000, 0x00000000, 0x00000000},
     {true,
      0x3F800000,
      {{"rn", 0x3F800001},
       {"rz", 0x3F800000},
       {"tp_rz", 0x3F800000},
       {"seq_rz", 0x3F800000},
       {"win_rz", 0x3F800000}}},
     {true,
      0x3F800000,
      {{"rn", 0x3F800001},
       {"rz", 0x3F800000},
       {"tp_rz", 0x3F800000},
       {"seq_rz", 0x3F800000},
       {"win_rz", 0x3F800000}}},
     {true,
      0x3F800000,
      {{"rn", 0x3F800001},
       {"rz", 0x3F800000},
       {"tp_rz", 0x3F800000},
       {"seq_rz", 0x3F800000},
       {"win_rz", 0x3F800000}}}},
    {"(a, b) . (1, 1), -0.75 ulp",
     {0xBF800000, 0xB3C00000, 0x00000000, 0x00000000},
     {0x3F800000, 0x3F800000, 0x00000000, 0x00000000},
     {true,
      0xBF800000,
      {{"rn", 0xBF800001},
       {"rz", 0xBF800000},
       {"tp_rz", 0xBF800000},
       {"seq_rz", 0xBF800000},
       {"win_rz", 0xBF800000}}},
     {true,
      0xBF800000,
      {{"rn", 0xBF800001},
       {"rz", 0xBF800000},
       {"tp_rz", 0xBF800000},
       {"seq_rz", 0xBF800000},
       {"win_rz", 0xBF800000}}},
     {true,
      0xBF800000,
      {{"rn", 0xBF800001},
       {"rz", 0xBF800000},
       {"tp_rz", 0xBF800000},
       {"seq_rz", 0xBF800000},
       {"win_rz", 0xBF800000}}}},
    {"(a, b) . (1, 1) + w(-c), +0.75 ulp",
     {0x3F800000, 0x33C00000, 0x00000000, 0x00000000},
     {0x3F800000, 0x3F800000, 0x00000000, 0xBF800001},
     {true,
      0x3F800000,
      {{"rn", 0x3F800001},
       {"rz", 0x3F800000},
       {"tp_rz", 0x3F800000},
       {"seq_rz", 0x3F800000},
       {"win_rz", 0x3F800000}}},
     {true,
      0xB3000000,
      {{"rn", 0xB3000000},
       {"rz", 0xB3000000},
       {"tp_rz", 0xB3000000},
       {"seq_rz", 0xB4000000},
       {"win_rz", 0xB3000000}}},
     {true,
      0x3F800000,
      {{"rn", 0x3F800001},
       {"rz", 0x3F800000},
       {"tp_rz", 0x3F800000},
       {"seq_rz", 0x3F800000},
       {"win_rz", 0x3F800000}}}},
    {"(a, RN(a * b), 0, NaN) . (b, -1, 0, NaN), Rugby",
     {0xBF80309E, 0x42203CC6, 0x00000000, 0x7FC00000},
     {0xC2200000, 0xBF800000, 0x00000000, 0x7FC00000},
     {true,
      0xB6800000,
      {{"rn", 0xB6000000},
       {"rz", 0xB6000000},
       {"tp_rz", 0xB6800000},
       {"seq_rz", 0xB6800000},
       {"win_rz", 0xB6800000}}},
     {true, 0x7FFFFFFF, {}},
     {true, 0x7FFFFFFF, {}}},
};

constexpr DotCase kAccumulationCases[] = {
    {"1 + small - 1, gap 2^28",
     {0x3F800000, 0x31800000, 0xBF800000, 0x00000000},
     {0x3F800000, 0x3F800000, 0x3F800000, 0x00000000},
     {true,
      0x31800000,
      {{"rn", 0x31800000},
       {"rz", 0x31800000},
       {"tp_rz", 0x31800000},
       {"seq_rz", 0x00000000},
       {"win_rz", 0x31800000}}},
     {true,
      0x31800000,
      {{"rn", 0x31800000},
       {"rz", 0x31800000},
       {"tp_rz", 0x31800000},
       {"seq_rz", 0x00000000},
       {"win_rz", 0x31800000}}},
     {true,
      0x31800000,
      {{"rn", 0x31800000},
       {"rz", 0x31800000},
       {"tp_rz", 0x31800000},
       {"seq_rz", 0x00000000},
       {"win_rz", 0x31800000}}}},
    {"big - big + small, gap 2^64",
     {0x4F800000, 0xCF800000, 0x2F800000, 0x00000000},
     {0x3F800000, 0x3F800000, 0x3F800000, 0x00000000},
     {true,
      0x00000000,
      {{"rn", 0x2F800000},
       {"rz", 0x2F800000},
       {"tp_rz", 0x2F800000},
       {"seq_rz", 0x2F800000},
       {"win_rz", 0x00000000}}},
     {true,
      0x00000000,
      {{"rn", 0x2F800000},
       {"rz", 0x2F800000},
       {"tp_rz", 0x2F800000},
       {"seq_rz", 0x2F800000},
       {"win_rz", 0x00000000}}},
     {true,
      0x00000000,
      {{"rn", 0x2F800000},
       {"rz", 0x2F800000},
       {"tp_rz", 0x2F800000},
       {"seq_rz", 0x2F800000},
       {"win_rz", 0x00000000}}}},
    {"(1, -(2^-24 + 2^-29)) . (1, 1)",
     {0x3F800000, 0xB3840000, 0x00000000, 0x00000000},
     {0x3F800000, 0x3F800000, 0x00000000, 0x00000000},
     {true,
      0x3F7FFFFF,
      {{"rn", 0x3F7FFFFF},
       {"rz", 0x3F7FFFFE},
       {"tp_rz", 0x3F7FFFFE},
       {"seq_rz", 0x3F7FFFFE},
       {"win_rz", 0x3F7FFFFF}}},
     {true,
      0x3F7FFFFF,
      {{"rn", 0x3F7FFFFF},
       {"rz", 0x3F7FFFFE},
       {"tp_rz", 0x3F7FFFFE},
       {"seq_rz", 0x3F7FFFFE},
       {"win_rz", 0x3F7FFFFF}}},
     {true,
      0x3F7FFFFF,
      {{"rn", 0x3F7FFFFF},
       {"rz", 0x3F7FFFFE},
       {"tp_rz", 0x3F7FFFFE},
       {"seq_rz", 0x3F7FFFFE},
       {"win_rz", 0x3F7FFFFF}}}},
};

constexpr uint32_t kCasesPerDraw = 4;

bool RunDotCases(const DotCase* cases, size_t count) {
  bool pass = true;
  for (size_t first = 0; first < count; first += kCasesPerDraw) {
    const size_t batch = std::min(count - first, size_t{kCasesPerDraw});

    VshConstant inputs[kCasesPerDraw * 2] = {};
    for (uint32_t i = 0; i < std::size(inputs); ++i) {
      inputs[i].index = 96 + i;
    }
    for (size_t i = 0; i < batch; ++i) {
      for (uint32_t j = 0; j < 4; ++j) {
        inputs[i * 2].bits[j] = cases[first + i].a[j];
        inputs[i * 2 + 1].bits[j] = cases[first + i].b[j];
      }
    }

    uint32_t outputs[3][4];
    if (!VshCompute(ComputeShader::kRoundDot, inputs, std::size(inputs), 188, 3,
                    outputs)) {
      TEST_FAIL("Compute draw failed.");
      return false;
    }

    for (size_t i = 0; i < batch; ++i) {
      const auto& c = cases[first + i];
      pass &= VshReportRounding("DP3", c.label, outputs[0][i], c.dp3);
      pass &= VshReportRounding("DPH", c.label, outputs[1][i], c.dph);
      pass &= VshReportRounding("DP4", c.label, outputs[2][i], c.dp4);
    }
  }
  return pass;
}

}  // namespace

bool TestVshRoundingDot::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return RunDotCases(kCases, std::size(kCases));
}

bool TestVshRoundingDotAccumulation::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return RunDotCases(kAccumulationCases, std::size(kAccumulationCases));
}
