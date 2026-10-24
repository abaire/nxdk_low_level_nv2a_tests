#include "vsh_rounding_common.h"

#include <algorithm>
#include <iterator>
#include <string>

static constexpr uint32_t kBinaryOutputs = 4;
static constexpr size_t kBinaryCasesPerDraw = kBinaryOutputs * 4;

bool VshReportRounding(const char* op, const char* label, uint32_t actual,
                       const RoundingExpectation& expectation) {
  LogFields fields =
      TestFields("op", op, "label", label, "actual", Hex32(actual));
  if (expectation.checked) {
    fields.Add("expected", Hex32(expectation.expected));
  }

  std::string matches;
  for (const auto& model : expectation.models) {
    if (!model.name) {
      break;
    }
    fields.Add(model.name, Hex32(model.bits));
    if (model.bits == actual) {
      if (!matches.empty()) {
        matches += ",";
      }
      matches += model.name;
    }
  }
  fields.Add("matches", matches);

  const auto actual_ul = static_cast<unsigned long>(actual);
  if (!expectation.checked) {
    TEST_INFO_EX(fields, "%s %s: 0x%08lX (%g) matches [%s]", op, label,
                 actual_ul, BitsFloat(actual), matches.c_str());
    return true;
  }

  if (actual == expectation.expected) {
    TEST_PASS_EX(fields, "%s %s: 0x%08lX matches [%s]", op, label, actual_ul,
                 matches.c_str());
    return true;
  }

  TEST_FAIL_EX(fields, "%s %s: 0x%08lX (%g), expected 0x%08lX, matches [%s]",
               op, label, actual_ul, BitsFloat(actual),
               static_cast<unsigned long>(expectation.expected),
               matches.c_str());
  return false;
}

bool VshRunBinaryRoundingCases(ComputeShader shader, const char* op,
                               const BinaryRoundingCase* cases, size_t count) {
  bool pass = true;
  for (size_t first = 0; first < count; first += kBinaryCasesPerDraw) {
    const size_t batch = std::min(count - first, kBinaryCasesPerDraw);

    VshConstant inputs[kBinaryOutputs * 2] = {};
    for (uint32_t i = 0; i < std::size(inputs); ++i) {
      inputs[i].index = 96 + i;
    }
    for (size_t i = 0; i < batch; ++i) {
      const auto& c = cases[first + i];
      inputs[(i / 4) * 2].bits[i % 4] = c.a;
      inputs[(i / 4) * 2 + 1].bits[i % 4] = c.b;
    }

    uint32_t outputs[kBinaryOutputs][4];
    if (!VshCompute(shader, inputs, std::size(inputs), 188, kBinaryOutputs,
                    outputs)) {
      TEST_FAIL("Compute draw failed.");
      return false;
    }

    for (size_t i = 0; i < batch; ++i) {
      const auto& c = cases[first + i];
      pass &=
          VshReportRounding(op, c.label, outputs[i / 4][i % 4], c.expectation);
    }
  }
  return pass;
}
