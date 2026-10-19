#include "test_vsh_americas_army.h"

#include <iterator>

#include "vsh_test_common.h"

bool TestVshAmericasArmy::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  const VshConstant inputs[] = {
      MakeConstant(96, 0.1647059f, 0.1647059f, 0.1686275f, 1.0f),
      MakeConstant(97, -728.0f, -4058.0f, 0.0f, 0.0f),
      MakeConstant(121, 2.0f, 2.0f, 2.0f, 1.0f),
      MakeConstant(135, 0.0f, 0.5f, 1.0f, 3.0f),
      MakeConstant(136, 0.9983897f, -0.0101479f, -0.0558118f, 0.0f),
      MakeConstant(137, -0.0567268f, -0.178603f, -0.9822846f, 0.0f),
      MakeConstant(138, 0.0f, 0.9838689f, -0.1788911f, 0.0f),
      MakeConstant(139, 31.5401363f, -20.6333656f, 67.6486282f, 1.0f),
      MakeConstant(140, 0.0f, 0.0f, 0.0f, 0.0f),
      MakeConstant(141, 0.0f, 0.0f, 0.0f, 0.0f),
      MakeConstant(142, 0.0f, 0.0f, 0.0f, 0.0f),
      MakeConstant(143, 0.0f, 0.0f, 0.0f, 0.0f),
      MakeConstant(144, 0.0f, 0.0f, 0.0f, 0.0f),
      MakeConstant(145, 1.0f, 0.0f, 0.0f, 0.0f),
      MakeConstant(146, 0.0f, 1.0f, 0.0f, 0.0f),
      MakeConstant(147, 0.0f, 0.0f, 1.0f, 0.0f),
  };

  uint32_t outputs[1][4];
  if (!VshCompute(ComputeShader::kAmericasArmy, inputs, std::size(inputs), 188,
                  1, outputs)) {
    TEST_FAIL("Compute draw failed.");
    return false;
  }

  using E = ExpectedComponent;
  const E expected[4] = {E::Bits(0x00000000), E::Bits(0x00000000),
                         E::Bits(0x00000000), E::Bits(0x7F800000)};
  return VshCheckVector("(Inf * 0).xyz, Inf", outputs[0], expected);
}
