#include "test_vsh_paired_ilu.h"

#include <iterator>

#include "vsh_test_common.h"

bool TestVshPairedIlu::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  static constexpr float kInput[4] = {25.0f, 1.123f, 2.123f, 3.123f};
  const VshConstant inputs[] = {
      MakeConstant(96, kInput[0], kInput[1], kInput[2], kInput[3]),
  };

  uint32_t outputs[4][4];
  if (!VshCompute(ComputeShader::kPairedIlu, inputs, std::size(inputs), 188, 4,
                  outputs)) {
    TEST_FAIL("Compute draw failed.");
    return false;
  }

  static constexpr uint32_t kRsq25 = 0x3E4CCCCC;
  using E = ExpectedComponent;
  const E input[4] = {E::Float(kInput[0]), E::Float(kInput[1]),
                      E::Float(kInput[2]), E::Float(kInput[3])};
  const E rsq_only[4] = {E::Bits(kRsq25), E::Ignore(), E::Ignore(),
                         E::Ignore()};
  const E rsq_in_r1[4] = {E::Bits(kRsq25), E::Float(kInput[1]),
                          E::Float(kInput[2]), E::Float(kInput[3])};

  bool pass = VshCheckVector("R1 after rsq r1.x", outputs[0], rsq_only);
  pass &= VshCheckVector("R1 after reset", outputs[1], input);
  pass &= VshCheckVector("R1 after rsq r10.x", outputs[2], rsq_in_r1);
  pass &= VshCheckVector("R10 after rsq r10.x", outputs[3], input);
  return pass;
}
