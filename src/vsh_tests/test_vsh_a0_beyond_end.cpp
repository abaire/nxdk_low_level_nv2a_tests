#include "test_vsh_a0_beyond_end.h"

#include <iterator>

#include "vsh_test_common.h"

bool TestVshA0BeyondEnd::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  static constexpr float kA0Values[] = {
      0,   1,   95,  96,   190,  191,  192,   193,   200,
      255, 256, 257, 383,  384,  447,  448,   511,   512,
      513, 767, 768, 1000, 4095, 4096, 65535, 65536, 1000000,
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
