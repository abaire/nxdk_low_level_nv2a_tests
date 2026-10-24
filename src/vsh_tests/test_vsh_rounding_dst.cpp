#include "test_vsh_rounding_dst.h"

#include <algorithm>
#include <iterator>

#include "vsh_rounding_common.h"

static constexpr BinaryRoundingCase kCases[] = {
    {"+0.75 ulp",
     0x3F800800,
     0x3F800C00,
     {true, 0x3F801400, {{"rn", 0x3F801401}, {"rz", 0x3F801400}}}},
    {"-0.75 ulp",
     0xBF800800,
     0x3F800C00,
     {true, 0xBF801400, {{"rn", 0xBF801401}, {"rz", 0xBF801400}}}},
};

static constexpr uint32_t kOutputs = 4;

bool TestVshRoundingDst::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  bool pass = true;
  for (size_t first = 0; first < std::size(kCases); first += kOutputs) {
    const size_t batch = std::min(std::size(kCases) - first, size_t{kOutputs});

    VshConstant inputs[kOutputs * 2] = {};
    for (uint32_t i = 0; i < std::size(inputs); ++i) {
      inputs[i].index = 96 + i;
    }
    for (size_t i = 0; i < batch; ++i) {
      inputs[i * 2].bits[1] = kCases[first + i].a;
      inputs[i * 2 + 1].bits[1] = kCases[first + i].b;
    }

    uint32_t outputs[kOutputs][4];
    if (!VshCompute(ComputeShader::kRoundDst, inputs, std::size(inputs), 188,
                    kOutputs, outputs)) {
      TEST_FAIL("Compute draw failed.");
      return false;
    }

    for (size_t i = 0; i < batch; ++i) {
      const auto& c = kCases[first + i];
      pass &= VshReportRounding("DST.y", c.label, outputs[i][1], c.expectation);
    }
  }
  return pass;
}
