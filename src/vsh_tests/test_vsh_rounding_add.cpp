#include "test_vsh_rounding_add.h"

#include <iterator>

#include "vsh_rounding_common.h"

static constexpr BinaryRoundingCase kCases[] = {
    {"+0.75 ulp",
     0x3F800000,
     0x33C00000,
     {true,
      0x3F800000,
      {{"rn", 0x3F800001}, {"rz", 0x3F800000}, {"win_rz", 0x3F800000}}}},
    {"-0.75 ulp",
     0xBF800000,
     0xB3C00000,
     {true,
      0xBF800000,
      {{"rn", 0xBF800001}, {"rz", 0xBF800000}, {"win_rz", 0xBF800000}}}},
    {"1 - 2^-28",
     0x3F800000,
     0xB1800000,
     {true,
      0x3F7FFFFF,
      {{"rn", 0x3F800000}, {"rz", 0x3F7FFFFF}, {"win_rz", 0x3F7FFFFF}}}},
    {"1 - (2^-24 + 2^-29)",
     0x3F800000,
     0xB3840000,
     {true,
      0x3F7FFFFF,
      {{"rn", 0x3F7FFFFF}, {"rz", 0x3F7FFFFE}, {"win_rz", 0x3F7FFFFF}}}},
};

bool TestVshRoundingAdd::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return VshRunBinaryRoundingCases(ComputeShader::kRoundAdd, "ADD", kCases,
                                   std::size(kCases));
}
