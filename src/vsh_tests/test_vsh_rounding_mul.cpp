#include "test_vsh_rounding_mul.h"

#include <iterator>

#include "vsh_rounding_common.h"

static constexpr BinaryRoundingCase kCases[] = {
    {"+0.75 ulp",
     0x3F800800,
     0x3F800C00,
     {true, 0x3F801400, {{"rn", 0x3F801401}, {"rz", 0x3F801400}}}},
    {"-0.75 ulp",
     0xBF800800,
     0x3F800C00,
     {true, 0xBF801400, {{"rn", 0xBF801401}, {"rz", 0xBF801400}}}},
};

bool TestVshRoundingMul::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return VshRunBinaryRoundingCases(ComputeShader::kRoundMul, "MUL", kCases,
                                   std::size(kCases));
}
