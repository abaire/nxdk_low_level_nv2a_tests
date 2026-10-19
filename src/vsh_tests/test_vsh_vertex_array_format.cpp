#include "test_vsh_vertex_array_format.h"

#include <pbkit/pbkit.h>

#include <cstdio>
#include <iterator>

#include "vsh_test_common.h"

namespace {

using E = ExpectedComponent;

bool RunV1(const char* label, uint32_t type, const void* data, uint32_t stride,
           const uint32_t expected_bits[4]) {
  const ArrayAttribute v1 = {type, 4, stride, data};
  uint32_t outputs[1][4];
  if (!VshComputeArray(ComputeShader::kV1Passthrough, nullptr, 0, v1, 188, 1,
                       outputs)) {
    TEST_FAIL_EX(TestFields("label", label), "%s: compute draw failed.", label);
    return false;
  }
  const E expected[4] = {E::Bits(expected_bits[0]), E::Bits(expected_bits[1]),
                         E::Bits(expected_bits[2]), E::Bits(expected_bits[3])};
  return VshCheckVector(label, outputs[0], expected);
}

struct ShortCase {
  int16_t values[4];
  uint32_t expected[4];
};

bool RunShorts(uint32_t type, const ShortCase* cases, size_t count) {
  bool pass = true;
  for (size_t i = 0; i < count; ++i) {
    const int16_t* v = cases[i].values;
    char label[48];
    snprintf(label, sizeof(label), "%d, %d, %d, %d", v[0], v[1], v[2], v[3]);
    pass &= RunV1(label, type, v, sizeof(cases[i].values), cases[i].expected);
  }
  return pass;
}

}  // namespace

bool TestVshVertexArrayFormatF::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  struct Case {
    const char* label;
    uint32_t bits[4];
  };
  static constexpr Case kCases[] = {
      {"0, 1, -1, 0.99999999",
       {0x00000000, 0x3F800000, 0xBF800000, 0x3F800000}},
      {"Inf, -Inf, NaN, -NaN",
       {0x7F800000, 0xFF800000, 0x7FC00000, 0xFFC00000}},
      {"Max, -Max, Min, -Min",
       {0x7F7FFFFF, 0xFF7FFFFF, 0x00800000, 0x80800000}},
      {"MaxSub, -MaxSub, MinSub, -MinSub",
       {0x007FFFFF, 0x807FFFFF, 0x00000001, 0x80000001}},
  };

  bool pass = true;
  for (const auto& c : kCases) {
    pass &= RunV1(c.label, NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F, c.bits,
                  sizeof(c.bits), c.bits);
  }
  return pass;
}

bool TestVshVertexArrayFormatS1::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  static constexpr ShortCase kCases[] = {
      {{32767, -32768, 256, -256},
       {0x3F800000, 0xBF800000, 0x3C004080, 0xBBFF80FF}},
      {{1, 0, -1, -32767}, {0x384000C0, 0x37800080, 0xB7800080, 0xBF7FFDFF}},
  };
  return RunShorts(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S1, kCases,
                   std::size(kCases));
}

bool TestVshVertexArrayFormatS32K::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  static constexpr ShortCase kCases[] = {
      {{32767, -32768, 256, -256},
       {0x46FFFE00, 0xC7000000, 0x43800000, 0xC3800000}},
      {{1, 0, -1, -32767}, {0x3F800000, 0x00000000, 0xBF800000, 0xC6FFFE00}},
  };
  return RunShorts(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S32K, kCases,
                   std::size(kCases));
}
