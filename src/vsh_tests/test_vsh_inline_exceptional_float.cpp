#include "test_vsh_inline_exceptional_float.h"

#include "vsh_test_common.h"

namespace {

struct ValueSet {
  const char* name;
  uint32_t values[4];
};

constexpr ValueSet kValueSets[] = {
    {"Inf, -Inf, NaN, -NaN", {0x7F800000, 0xFF800000, 0x7FC00000, 0xFFC00000}},
    {"Max, -Max, Min, -Min", {0x7F7FFFFF, 0xFF7FFFFF, 0x00800000, 0x80800000}},
    {"MaxSub, -MaxSub, MinSub, -MinSub",
     {0x007FFFFF, 0x807FFFFF, 0x00000001, 0x80000001}},
};

}  // namespace

bool TestVshInlineExceptionalFloat::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  using E = ExpectedComponent;
  bool pass = true;
  for (const auto& set : kValueSets) {
    TEST_SCOPE(set.name);
    const uint32_t* v = set.values;
    const InlineAttributes attributes = {
        {v[0], v[1], v[2], v[3]},
        {v[0], v[1], v[2]},
        {v[0], v[1], v[2], v[3]},
        {v[0], v[1], v[2], v[3]},
        v[0],
    };

    uint32_t outputs[5][4];
    if (!VshComputeInline(ComputeShader::kInlineAttributes, attributes, 187, 5,
                          outputs)) {
      TEST_FAIL("Compute draw failed.");
      return false;
    }

    const E all[4] = {E::Bits(v[0]), E::Bits(v[1]), E::Bits(v[2]),
                      E::Bits(v[3])};
    const E xyz[4] = {E::Bits(v[0]), E::Bits(v[1]), E::Bits(v[2]), E::Ignore()};
    const E x[4] = {E::Bits(v[0]), E::Ignore(), E::Ignore(), E::Ignore()};

    pass &= VshCheckVector("v0 (SET_VERTEX4F)", outputs[0], all);
    pass &= VshCheckVector("v2 (SET_NORMAL3F)", outputs[1], xyz);
    pass &= VshCheckVector("v3 (SET_DIFFUSE_COLOR4F)", outputs[2], all);
    pass &= VshCheckVector("v4 (SET_SPECULAR_COLOR4F)", outputs[3], all);
    pass &= VshCheckVector("v5 (SET_FOG_COORD)", outputs[4], x);
  }
  return pass;
}
