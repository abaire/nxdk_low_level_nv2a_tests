#include "test_vsh_spy_vs_spy_menu.h"

#include <pbkit/pbkit.h>

#include <iterator>

#include "vsh_test_common.h"

bool TestVshSpyVsSpyMenu::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  static constexpr float kC58[4] = {320.0f, -240.0f, 16777215.0f, 0.0f};
  static constexpr float kC59[4] = {320.03125f, 240.03125f, 0.0f, 0.0f};
  static constexpr float kC99[4] = {-16.8876857758f, -18.2681617737f,
                                    3.6487216949f, 1.0f};
  const VshConstant inputs[] = {
      MakeConstant(58, kC58[0], kC58[1], kC58[2], kC58[3]),
      MakeConstant(59, kC59[0], kC59[1], kC59[2], kC59[3]),
      MakeConstant(96, 1.0f, 0.0f, 0.0f, 0.0f),
      MakeConstant(97, 0.0f, 1.0f, 0.0f, 0.0f),
      MakeConstant(98, 0.0f, 0.0f, 1.0f, 0.0f),
      MakeConstant(99, kC99[0], kC99[1], kC99[2], kC99[3]),
      MakeConstant(120, 1.0f, 0.0000610352f, 0.0009765625f, 1.0f),
      MakeConstant(125, 1.0f, 1.0f, 1.0f, 0.0f),
  };

  static constexpr int16_t kV1[4] = {89, 5156, -151, 417};
  const ArrayAttribute v1 = {NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S32K, 4,
                             sizeof(kV1), kV1};

  uint32_t outputs[4][4];
  if (!VshComputeArray(ComputeShader::kSpyVsSpyMenu, inputs, std::size(inputs),
                       v1, 188, 4, outputs)) {
    TEST_FAIL("Compute draw failed.");
    return false;
  }

  using E = ExpectedComponent;
  const E expected_r12[4] = {E::Bits(0x439FDE1F), E::Bits(0x437CE37C),
                             E::Bits(0x46CDA4BB), E::Bits(0xC7BBF9CC)};
  const E expected_r1[4] = {E::Bits(0xB72E5201), E::Bits(0xB72E5201),
                            E::Bits(0xB72E5201), E::Bits(0xB72E5201)};
  const E expected_r11[4] = {E::Float(89.0f), E::Float(5156.0f),
                             E::Float(-151.0f), E::Ignore()};
  const E expected_r0[4] = {E::Float(1.0f), E::Float(89.0f), E::Float(5156.0f),
                            E::Float(-151.0f)};

  bool pass = VshCheckVector("R0", outputs[3], expected_r0);
  pass &= VshCheckVector("R11", outputs[2], expected_r11);
  pass &= VshCheckVector("R1.x = RCC(R12.w)", outputs[1], expected_r1);
  pass &= VshCheckVector("R12 after oPos MAD", outputs[0], expected_r12);
  return pass;
}
