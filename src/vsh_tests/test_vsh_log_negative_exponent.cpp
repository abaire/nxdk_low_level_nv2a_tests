#include "test_vsh_log_negative_exponent.h"

#include <iterator>

#include "vsh_test_common.h"

bool TestVshLogNegativeExponent::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  struct Probe {
    float input;
    uint32_t expected_red;
  };
  static constexpr Probe kProbes[] = {
      {0.75f, 0}, {0.5f, 0}, {0.25f, 0}, {0.001f, 0}, {1.0f, 128}, {2.0f, 255},
  };

  float inputs[std::size(kProbes)];
  for (size_t i = 0; i < std::size(kProbes); ++i) {
    inputs[i] = kProbes[i].input;
  }

  uint32_t rgb[std::size(kProbes)];
  uint32_t pgraph_intr = 0;
  if (!VshRunProbes(ProbeShader::kLogX, inputs, std::size(inputs), rgb,
                    &pgraph_intr)) {
    TEST_FAIL("Probe draw failed.");
    return false;
  }

  TEST_INFO_EX(TestFields("pgraph_intr", Hex32(pgraph_intr)),
               "NV_PGRAPH_INTR after draw: 0x%08X", pgraph_intr);

  bool pass = true;
  for (size_t i = 0; i < std::size(kProbes); ++i) {
    const uint32_t red = (rgb[i] >> 16) & 0xFF;
    const uint32_t expected = kProbes[i].expected_red;
    const bool ok = red + 1 >= expected && red <= expected + 1;
    const LogFields fields =
        TestFields("input", kProbes[i].input, "rgb", Hex32(rgb[i]), "red", red,
                   "expected_red", expected);
    if (ok) {
      TEST_PASS_EX(fields, "LOG(%f).x red=%u as expected", kProbes[i].input,
                   red);
    } else {
      TEST_FAIL_EX(fields, "LOG(%f).x red=%u, expected %u", kProbes[i].input,
                   red, expected);
      pass = false;
    }
  }
  return pass;
}
