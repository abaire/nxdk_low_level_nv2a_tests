#include "test_vsh_rugby_projection.h"

#include <pbkit/pbkit.h>

#include <iterator>

#include "vsh_rounding_common.h"

bool TestVshRugbyProjection::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  const VshConstant inputs[] = {
      MakeConstant(0, 1.0f, 0.0f, 0.0f, 0.0f),
      MakeConstant(1, 0.0f, 1.0f, 0.0f, 0.0f),
      {2, {0, 0, 0xBF80309E, 0xC2203CC6}},
      MakeConstant(3, 0.0f, 0.0f, -1.0f, 0.0f),
      MakeConstant(58, 320.0f, -240.0f, 16777215.0f, 0.0f),
      MakeConstant(59, 319.53125f, 239.53125f, 0.0f, 0.0f),
  };

  static constexpr float kV0[4] = {100.0f, 50.0f, -40.0f, 1.0f};
  const ArrayAttribute v1 = {NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F, 4,
                             sizeof(kV0), kV0};

  uint32_t outputs[4][4];
  if (!VshComputeArray(ComputeShader::kRugbyProjection, inputs,
                       std::size(inputs), v1, 188, 4, outputs)) {
    TEST_FAIL("Compute draw failed.");
    return false;
  }

  static constexpr RoundingExpectation kProjectedZ = {
      true,
      0xB6800000,
      {{"rn", 0xB6000000},
       {"rz", 0xB6000000},
       {"tp_rz", 0xB6800000},
       {"rn_products", 0x00000000}}};
  bool pass =
      VshReportRounding("DP4", "Rugby R12.z", outputs[0][2], kProjectedZ);

  using E = ExpectedComponent;
  const E expected_projected[4] = {E::Float(100.0f), E::Float(50.0f),
                                   E::Bits(0xB6800000), E::Float(40.0f)};
  const E expected_scaled[4] = {E::Float(32000.0f), E::Float(-12000.0f),
                                E::Bits(0xC27FFFFF), E::Float(40.0f)};
  pass &= VshCheckVector("R12 after DP4", outputs[0], expected_projected);
  pass &= VshCheckVector("R12 after MUL", outputs[1], expected_scaled);

  const E expected_rcc[4] = {E::Bits(0x3CCCCCCC), E::Ignore(), E::Ignore(),
                             E::Ignore()};
  const E expected_final[4] = {E::Bits(0x448BF0FF), E::Bits(0xC271DFF8),
                               E::Bits(0xBFCCCCCB), E::Float(40.0f)};
  pass &= VshCheckVector("R1.x = RCC(40)", outputs[2], expected_rcc);
  pass &= VshCheckVector("R12 after MAD", outputs[3], expected_final);
  return pass;
}
