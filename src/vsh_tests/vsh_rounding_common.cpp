#include "vsh_rounding_common.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <string>

#include "printf/printf.h"

static constexpr uint32_t kBinaryOutputs = 4;
static constexpr size_t kBinaryCasesPerDraw = kBinaryOutputs * 4;

static void AppendHex32(std::string& out, uint32_t value) {
  char buf[16];
  snprintf_(buf, sizeof(buf), "0x%08X", static_cast<unsigned int>(value));
  out += buf;
}

static void AddContextFields(LogFields& fields, const char* op,
                             const char* label,
                             const RoundingExpectation& expectation,
                             const VshRecordContext& context) {
  fields.Add("group", context.group);
  fields.Add("record_op", context.record_op ? context.record_op : op);
  fields.Add("case", context.case_label ? context.case_label : label);
  if (context.component) {
    fields.Add("component", context.component);
  }

  for (const auto& operand : context.operands) {
    if (!operand.name) {
      break;
    }
    std::string value;
    for (uint32_t i = 0; i < operand.count; ++i) {
      if (i) {
        value += ",";
      }
      AppendHex32(value, operand.bits[i]);
    }
    fields.Add(operand.name, std::move(value));
  }

  std::string models;
  for (const auto& model : expectation.models) {
    if (!model.name) {
      break;
    }
    if (!strcmp(model.name, "hw")) {
      continue;
    }
    if (!models.empty()) {
      models += ",";
    }
    models += model.name;
    models += "=";
    AppendHex32(models, model.bits);
  }
  fields.Add("models", std::move(models));
}

bool VshReportRounding(const char* op, const char* label, uint32_t actual,
                       const RoundingExpectation& expectation,
                       const VshRecordContext* context) {
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
  if (context) {
    AddContextFields(fields, op, label, expectation, *context);
  }

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
                               const BinaryRoundingCase* cases, size_t count,
                               const char* group) {
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
      const VshRecordContext context = {group,
                                        nullptr,
                                        nullptr,
                                        nullptr,
                                        {{"in_a", &c.a, 1}, {"in_b", &c.b, 1}}};
      pass &= VshReportRounding(op, c.label, outputs[i / 4][i % 4],
                                c.expectation, group ? &context : nullptr);
    }
  }
  return pass;
}
