#include "test_vsh_mac_add.h"

#include <iterator>

#include "vsh_test_common.h"

bool TestVshMacAdd::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  static constexpr float kA[4] = {1.0f, 2.0f, -1.0f, -2.33f};
  static constexpr float kB[4] = {1000.5f, 2424.99f, 1.0f, -100.0f};
  const VshConstant inputs[] = {
      MakeConstant(96, kA[0], kA[1], kA[2], kA[3]),
      MakeConstant(97, kB[0], kB[1], kB[2], kB[3]),
  };

  uint32_t outputs[1][4];
  if (!VshCompute(ComputeShader::kMacAdd, inputs, std::size(inputs), 188, 1,
                  outputs)) {
    TEST_FAIL("Compute draw failed.");
    return false;
  }

  using E = ExpectedComponent;
  const E expected[4] = {E::Bits(0x447A6000), E::Bits(0x4517AFD7),
                         E::Bits(0x00000000), E::Bits(0xC2CCA8F5)};
  return VshCheckVector("add c[96], c[97]", outputs[0], expected);
}
