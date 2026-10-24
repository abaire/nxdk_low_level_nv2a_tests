#include "test_vsh_rounding_mad.h"

#include <algorithm>
#include <iterator>

#include "vsh_rounding_common.h"

namespace {

struct MadCase {
  const char* label;
  uint32_t a;
  uint32_t b;
  uint32_t c;
  RoundingExpectation expectation;
};

constexpr MadCase kCases[] = {
    {"a * b - RN(a * b), +0.75 ulp",
     0x3F800800,
     0x3F800C00,
     0xBF801401,
     {true,
      0xB4000000,
      {{"rn", 0xB3000000},
       {"rz", 0xB3000000},
       {"rn_rn", 0x00000000},
       {"rz_rz", 0xB4000000},
       {"win_rz", 0xB4000000}}}},
    {"a * 1 + b, +0.75 ulp",
     0x3F800000,
     0x3F800000,
     0x33C00000,
     {true,
      0x3F800000,
      {{"rn", 0x3F800001},
       {"rz", 0x3F800000},
       {"rn_rn", 0x3F800001},
       {"rz_rz", 0x3F800000},
       {"win_rz", 0x3F800000}}}},
    {"both inexact, -",
     0xBF800800,
     0x3F800C00,
     0xB3800000,
     {true,
      0xBF801400,
      {{"rn", 0xBF801401},
       {"rz", 0xBF801401},
       {"rn_rn", 0xBF801402},
       {"rz_rz", 0xBF801400},
       {"win_rz", 0xBF801400}}}},
    {"1 * 1 - 2^-28",
     0x3F800000,
     0x3F800000,
     0xB1800000,
     {true,
      0x3F7FFFFF,
      {{"rn", 0x3F800000},
       {"rz", 0x3F7FFFFF},
       {"rn_rn", 0x3F800000},
       {"rz_rz", 0x3F7FFFFF},
       {"win_rz", 0x3F7FFFFF}}}},
    {"1 * 1 - (2^-24 + 2^-29)",
     0x3F800000,
     0x3F800000,
     0xB3840000,
     {true,
      0x3F7FFFFF,
      {{"rn", 0x3F7FFFFF},
       {"rz", 0x3F7FFFFE},
       {"rn_rn", 0x3F7FFFFF},
       {"rz_rz", 0x3F7FFFFE},
       {"win_rz", 0x3F7FFFFF}}}},
};

constexpr uint32_t kOutputs = 4;
constexpr size_t kCasesPerDraw = kOutputs * 4;

}  // namespace

bool TestVshRoundingMad::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  bool pass = true;
  for (size_t first = 0; first < std::size(kCases); first += kCasesPerDraw) {
    const size_t batch = std::min(std::size(kCases) - first, kCasesPerDraw);

    VshConstant inputs[kOutputs * 3] = {};
    for (uint32_t i = 0; i < std::size(inputs); ++i) {
      inputs[i].index = 96 + i;
    }
    for (size_t i = 0; i < batch; ++i) {
      const auto& c = kCases[first + i];
      inputs[(i / 4) * 3].bits[i % 4] = c.a;
      inputs[(i / 4) * 3 + 1].bits[i % 4] = c.b;
      inputs[(i / 4) * 3 + 2].bits[i % 4] = c.c;
    }

    uint32_t outputs[kOutputs][4];
    if (!VshCompute(ComputeShader::kRoundMad, inputs, std::size(inputs), 188,
                    kOutputs, outputs)) {
      TEST_FAIL("Compute draw failed.");
      return false;
    }

    for (size_t i = 0; i < batch; ++i) {
      const auto& c = kCases[first + i];
      pass &= VshReportRounding("MAD", c.label, outputs[i / 4][i % 4],
                                c.expectation);
    }
  }
  return pass;
}
