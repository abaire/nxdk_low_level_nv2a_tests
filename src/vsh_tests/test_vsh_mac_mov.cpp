#include "test_vsh_mac_mov.h"

#include <iterator>

#include "vsh_test_common.h"

bool TestVshMacMov::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  static constexpr float kInput[4] = {1.0f, 2.0f, -3.0f, -4.12345f};
  const VshConstant inputs[] = {
      MakeConstant(96, kInput[0], kInput[1], kInput[2], kInput[3]),
  };

  uint32_t outputs[2][4];
  if (!VshCompute(ComputeShader::kMacMov, inputs, std::size(inputs), 188, 2,
                  outputs)) {
    TEST_FAIL("Compute draw failed.");
    return false;
  }

  using E = ExpectedComponent;
  const E full[4] = {E::Float(kInput[0]), E::Float(kInput[1]),
                     E::Float(kInput[2]), E::Float(kInput[3])};
  const E masked[4] = {E::Float(kInput[0]), E::Float(kInput[1]),
                       E::Bits(kVshComputeSentinel),
                       E::Bits(kVshComputeSentinel)};

  bool pass = VshCheckVector("mov c[188], c[96]", outputs[0], full);
  pass &= VshCheckVector("mov c[189].xy, c[96]", outputs[1], masked);
  return pass;
}
