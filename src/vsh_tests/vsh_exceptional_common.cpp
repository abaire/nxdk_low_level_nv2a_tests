#include "vsh_exceptional_common.h"

#include <algorithm>
#include <iterator>
#include <string>

bool VshReportVector(const char* op, const char* label,
                     const uint32_t actual[4],
                     const RoundingExpectation expected[4],
                     const VshRecordContext& context) {
  static constexpr const char* kComponents[] = {"x", "y", "z", "w"};
  bool pass = true;
  VshRecordContext component_context = context;
  component_context.case_label = label;
  for (uint32_t i = 0; i < 4; ++i) {
    const std::string component_label =
        std::string(label) + "." + kComponents[i];
    component_context.component = kComponents[i];
    pass &= VshReportRounding(op, component_label.c_str(), actual[i],
                              expected[i], &component_context);
  }
  return pass;
}

bool VshRunTernaryCases(ComputeShader shader, const char* op,
                        const TernaryCase* cases, size_t count) {
  static constexpr uint32_t kOutputs = 4;
  static constexpr size_t kCasesPerDraw = kOutputs * 4;

  bool pass = true;
  for (size_t first = 0; first < count; first += kCasesPerDraw) {
    const size_t batch = std::min(count - first, kCasesPerDraw);

    VshConstant inputs[kOutputs * 3] = {};
    for (uint32_t i = 0; i < std::size(inputs); ++i) {
      inputs[i].index = 96 + i;
    }
    for (size_t i = 0; i < batch; ++i) {
      const auto& c = cases[first + i];
      inputs[(i / 4) * 3].bits[i % 4] = c.a;
      inputs[(i / 4) * 3 + 1].bits[i % 4] = c.b;
      inputs[(i / 4) * 3 + 2].bits[i % 4] = c.c;
    }

    uint32_t outputs[kOutputs][4];
    if (!VshCompute(shader, inputs, std::size(inputs), 188, kOutputs,
                    outputs)) {
      TEST_FAIL("Compute draw failed.");
      return false;
    }

    for (size_t i = 0; i < batch; ++i) {
      const auto& c = cases[first + i];
      const VshRecordContext context = {
          "mac",
          nullptr,
          nullptr,
          nullptr,
          {{"in_a", &c.a, 1}, {"in_b", &c.b, 1}, {"in_c", &c.c, 1}}};
      pass &= VshReportRounding(op, c.label, outputs[i / 4][i % 4],
                                c.expectation, &context);
    }
  }
  return pass;
}

bool VshRunExceptionalDotCases(const ExceptionalDotCase* cases, size_t count) {
  static constexpr size_t kCasesPerDraw = 4;

  bool pass = true;
  for (size_t first = 0; first < count; first += kCasesPerDraw) {
    const size_t batch = std::min(count - first, kCasesPerDraw);

    VshConstant inputs[kCasesPerDraw * 2] = {};
    for (uint32_t i = 0; i < std::size(inputs); ++i) {
      inputs[i].index = 96 + i;
    }
    for (size_t i = 0; i < batch; ++i) {
      for (uint32_t j = 0; j < 4; ++j) {
        inputs[i * 2].bits[j] = cases[first + i].a[j];
        inputs[i * 2 + 1].bits[j] = cases[first + i].b[j];
      }
    }

    uint32_t outputs[3][4];
    if (!VshCompute(ComputeShader::kRoundDot, inputs, std::size(inputs), 188, 3,
                    outputs)) {
      TEST_FAIL("Compute draw failed.");
      return false;
    }

    for (size_t i = 0; i < batch; ++i) {
      const auto& c = cases[first + i];
      const VshRecordContext context = {"mac",
                                        nullptr,
                                        nullptr,
                                        nullptr,
                                        {{"in_a", c.a, 4}, {"in_b", c.b, 4}}};
      pass &= VshReportRounding("DP3", c.label, outputs[0][i], c.dp3, &context);
      pass &= VshReportRounding("DPH", c.label, outputs[1][i], c.dph, &context);
      pass &= VshReportRounding("DP4", c.label, outputs[2][i], c.dp4, &context);
    }
  }
  return pass;
}

static constexpr size_t kIluCasesPerDraw = 4;

template <typename Case, typename Report>
static bool RunIluCases(ComputeShader shader, const Case* cases, size_t count,
                        Report report) {
  bool pass = true;
  for (size_t first = 0; first < count; first += kIluCasesPerDraw) {
    const size_t batch = std::min(count - first, kIluCasesPerDraw);

    VshConstant input = {96, {}};
    for (size_t i = 0; i < batch; ++i) {
      input.bits[i] = cases[first + i].input;
    }

    uint32_t outputs[kIluCasesPerDraw][4];
    if (!VshCompute(shader, &input, 1, 188, kIluCasesPerDraw, outputs)) {
      TEST_FAIL("Compute draw failed.");
      return false;
    }

    for (size_t i = 0; i < batch; ++i) {
      pass &= report(cases[first + i], outputs[i]);
    }
  }
  return pass;
}

bool VshRunScalarIluCases(ComputeShader shader, const char* op,
                          const ScalarIluCase* cases, size_t count) {
  return RunIluCases(
      shader, cases, count,
      [op](const ScalarIluCase& c, const uint32_t* result) {
        const VshRecordContext context = {
            "ilu", nullptr, nullptr, nullptr, {{"in_a", &c.input, 1}}};
        return VshReportRounding(op, c.label, result[0], c.expectation,
                                 &context);
      });
}

bool VshRunVectorIluCases(ComputeShader shader, const char* op,
                          const VectorIluCase* cases, size_t count) {
  return RunIluCases(
      shader, cases, count,
      [op](const VectorIluCase& c, const uint32_t* result) {
        const VshRecordContext context = {
            "ilu", nullptr, nullptr, nullptr, {{"in_a", &c.input, 1}}};
        return VshReportVector(op, c.label, result, c.expectation, context);
      });
}
