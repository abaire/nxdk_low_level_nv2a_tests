#include "test_vsh_a0_offset_beyond_end.h"

#include <iterator>

#include "vsh_test_common.h"

bool TestVshA0OffsetBeyondEnd::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  static constexpr float kA0Values[] = {
      -191, -96, -1, 0, 1, 2, 9, 65, 256, 4096, 65536,
  };
  uint32_t rgb[std::size(kA0Values)];
  uint32_t pgraph_intr = 0;

  if (!VshRunProbes(ProbeShader::kOffset191, kA0Values, std::size(kA0Values),
                    rgb, &pgraph_intr)) {
    TEST_FAIL("Probe draw failed.");
    return false;
  }

  return VshReportA0Probes(kA0Values, std::size(kA0Values), 191, rgb,
                           pgraph_intr);
}
