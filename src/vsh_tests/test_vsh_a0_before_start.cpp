#include "test_vsh_a0_before_start.h"

#include <iterator>

#include "vsh_test_common.h"

bool TestVshA0BeforeStart::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  static constexpr float kA0Values[] = {
      0,    -1,   -2,   -96,  -191, -192,  -193,
      -256, -257, -384, -512, -513, -4096, -65536,
  };
  uint32_t rgb[std::size(kA0Values)];
  uint32_t pgraph_intr = 0;

  if (!VshRunProbes(ProbeShader::kOffset0, kA0Values, std::size(kA0Values), rgb,
                    &pgraph_intr)) {
    TEST_FAIL("Probe draw failed.");
    return false;
  }

  return VshReportA0Probes(kA0Values, std::size(kA0Values), 0, rgb,
                           pgraph_intr);
}
