#include "test_vsh_exceptional_paths.h"

#include <pbkit/pbkit.h>

#include <iterator>
#include <string>

#include "vsh_exceptional_common.h"

using namespace VshBits;

namespace {

constexpr uint32_t kPathOutputs = 8;

constexpr const char* kPathOps[kPathOutputs] = {
    "A * B", "B * A", "A * B + C", "B * A + C",
    "DOTS",  "A + B", "MIN(A, B)", "MAX(A, B)",
};

constexpr uint32_t kDotsOutput = 4;

//! Whether each output in kPathOps reads C.
constexpr bool kPathOpUsesC[kPathOutputs] = {false, false, true,  true,
                                             false, false, false, false};

//! The scalar result in each component of the DOTS output.
constexpr const char* kDotOps[4] = {"DP4(A, B)", "DP3(A, B)", "DPH(A, B)",
                                    "DPH(B, A)"};

struct PathCase {
  const char* label;
  uint32_t a[4];
  uint32_t b[4];
  uint32_t c[4];
  //! Indexed as kPathOps. DOTS is (DP4(A, B), DP3(A, B), DPH(A, B),
  //! DPH(B, A)).
  RoundingExpectation expected[kPathOutputs][4];
};

constexpr RoundingExpectation kZeroOrNaN = Expect(kPosZero, "ieee", kQNaN);
constexpr RoundingExpectation kCOrNaN = Expect(kOneAndHalf, "ieee", kQNaN);

constexpr PathCase kPathCases[] = {
    {"(0, -0, 0, denormal) . (+inf, +inf, 0x7FC00000, +inf)",
     {kPosZero, kNegZero, kPosZero, kDenormal},
     {kPosInf, kPosInf, kQNaN, kPosInf},
     {kOneAndHalf, kOneAndHalf, kOneAndHalf, kOneAndHalf},
     {
         {kZeroOrNaN, Expect(kPosZero, "xor_sign", kNegZero, "ieee", kQNaN),
          kZeroOrNaN, Expect(kPosZero, "ieee", kPosInf)},
         {kZeroOrNaN, Expect(kPosZero, "xor_sign", kNegZero, "ieee", kQNaN),
          kZeroOrNaN, Expect(kPosZero, "ieee", kPosInf)},
         {kCOrNaN, kCOrNaN, kCOrNaN, Expect(kOneAndHalf, "ieee", kPosInf)},
         {kCOrNaN, kCOrNaN, kCOrNaN, Expect(kOneAndHalf, "ieee", kPosInf)},
         {kZeroOrNaN, kZeroOrNaN, Expect(kPosInf, "ieee", kQNaN), kZeroOrNaN},
         {Expect(kPosInf), Expect(kPosInf), Expect(kNaN, "ieee", kQNaN),
          Expect(kPosInf)},
         {Expect(kPosZero), Expect(kNegZero), Expect(kPosZero, "d3d", kQNaN),
          Expect(kDenormal, "d3d", kPosZero)},
         {Expect(kPosInf), Expect(kPosInf), Expect(kQNaN, "canon_nan", kNaN),
          Expect(kPosInf)},
     }},
    {"(+inf, -inf, 0x7FC00000, 1.5) . (0, -0, 0, 0)",
     {kPosInf, kNegInf, kQNaN, kOneAndHalf},
     {kPosZero, kNegZero, kPosZero, kPosZero},
     {kOneAndHalf, kOneAndHalf, kOneAndHalf, kOneAndHalf},
     {
         {kZeroOrNaN, kZeroOrNaN, kZeroOrNaN, Expect(kPosZero)},
         {kZeroOrNaN, kZeroOrNaN, kZeroOrNaN, Expect(kPosZero)},
         {kCOrNaN, kCOrNaN, kCOrNaN, Expect(kOneAndHalf)},
         {kCOrNaN, kCOrNaN, kCOrNaN, Expect(kOneAndHalf)},
         {kZeroOrNaN, kZeroOrNaN, kZeroOrNaN, kCOrNaN},
         {Expect(kPosInf), Expect(kNegInf), Expect(kNaN, "ieee", kQNaN),
          Expect(kOneAndHalf)},
         {Expect(kPosZero), Expect(kNegInf), Expect(kPosZero, "nan_prop", kNaN),
          Expect(kPosZero)},
         {Expect(kPosInf), Expect(kNegZero), Expect(kQNaN, "d3d", kPosZero),
          Expect(kOneAndHalf)},
     }},
    {"(1.5, 2, -1.5, 3) . (2, 0.5, 2, 1)",
     {kOneAndHalf, kTwo, kNegOneAndHalf, kThree},
     {kTwo, kHalf, kTwo, kOne},
     {kOneAndHalf, kOneAndHalf, kOneAndHalf, kOneAndHalf},
     {
         {Expect(kThree), Expect(kOne), Expect(0xC0400000), Expect(kThree)},
         {Expect(kThree), Expect(kOne), Expect(0xC0400000), Expect(kThree)},
         {Expect(0x40900000), Expect(0x40200000), Expect(kNegOneAndHalf),
          Expect(0x40900000)},
         {Expect(0x40900000), Expect(0x40200000), Expect(kNegOneAndHalf),
          Expect(0x40900000)},
         {Expect(kFour), Expect(kOne), Expect(kTwo), Expect(kFour)},
         {Expect(0x40600000), Expect(0x40200000), Expect(kHalf), Expect(kFour)},
         {Expect(kOneAndHalf), Expect(kHalf), Expect(kNegOneAndHalf),
          Expect(kOne)},
         {Expect(kTwo), Expect(kTwo), Expect(kTwo), Expect(kThree)},
     }},
};

struct ChainCase {
  const char* label;
  uint32_t p[4];
  RoundingExpectation rcp_w;
  RoundingExpectation p_over_w[4];
  RoundingExpectation d2;
  RoundingExpectation rsq_d2;
  RoundingExpectation d;
};

constexpr RoundingExpectation kPosInfExpected = Expect(kPosInf);
constexpr RoundingExpectation kInfOrNaN = Expect(kPosInf, "ieee", kQNaN);

constexpr ChainCase kChainCases[] = {
    {"(1, 2, 3, 1)",
     {kOne, kTwo, kThree, kOne},
     Expect(kOne),
     {Expect(kOne), Expect(kTwo), Expect(kThree), Expect(kOne)},
     Expect(0x41600000),
     Expect(0x3E88D676),
     Expect(0x406F774E)},
    {"(1, 2, 3, +0)",
     {kOne, kTwo, kThree, kPosZero},
     kPosInfExpected,
     {kPosInfExpected, kPosInfExpected, kPosInfExpected, kZeroOrNaN},
     kPosInfExpected,
     Expect(kPosZero),
     kZeroOrNaN},
    {"(1, 2, 3, -0)",
     {kOne, kTwo, kThree, kNegZero},
     Expect(kNegInf, "pos_inf", kPosInf),
     {Expect(kNegInf, "pos_inf", kPosInf), Expect(kNegInf, "pos_inf", kPosInf),
      Expect(kNegInf, "pos_inf", kPosInf),
      Expect(kPosZero, "xor_sign", kNegZero, "ieee", kQNaN)},
     kPosInfExpected,
     Expect(kPosZero),
     kZeroOrNaN},
    {"(-1, 2, -3, +0)",
     {kNegOne, kTwo, 0xC0400000, kPosZero},
     kPosInfExpected,
     {Expect(kNegInf), kPosInfExpected, Expect(kNegInf), kZeroOrNaN},
     kPosInfExpected,
     Expect(kPosZero),
     kZeroOrNaN},
    {"(0, 0, 3, +0)",
     {kPosZero, kPosZero, kThree, kPosZero},
     kPosInfExpected,
     {kZeroOrNaN, kZeroOrNaN, kPosInfExpected, kZeroOrNaN},
     kInfOrNaN,
     kZeroOrNaN,
     kZeroOrNaN},
    {"(0, 0, 0, +0)",
     {kPosZero, kPosZero, kPosZero, kPosZero},
     kPosInfExpected,
     {kZeroOrNaN, kZeroOrNaN, kZeroOrNaN, kZeroOrNaN},
     kZeroOrNaN,
     kInfOrNaN,
     kZeroOrNaN},
    {"(-0, -0, -0, -0)",
     {kNegZero, kNegZero, kNegZero, kNegZero},
     Expect(kNegInf, "pos_inf", kPosInf),
     {kZeroOrNaN, kZeroOrNaN, kZeroOrNaN, kZeroOrNaN},
     kZeroOrNaN,
     kInfOrNaN,
     kZeroOrNaN},
};

bool ReportPathCase(const char* path, const PathCase& c,
                    const uint32_t (*outputs)[4]) {
  static constexpr const char* kComponents[] = {"x", "y", "z", "w"};

  bool pass = true;
  for (uint32_t i = 0; i < kPathOutputs; ++i) {
    const std::string op = std::string(path) + " " + kPathOps[i];
    VshRecordContext context = {"paths",
                                nullptr,
                                nullptr,
                                nullptr,
                                {{"in_a", c.a, 4}, {"in_b", c.b, 4}}};
    if (kPathOpUsesC[i]) {
      context.operands[2] = {"in_c", c.c, 4};
    }

    if (i != kDotsOutput) {
      pass &= VshReportVector(op.c_str(), c.label, outputs[i], c.expected[i],
                              context);
      continue;
    }

    context.case_label = c.label;
    for (uint32_t j = 0; j < 4; ++j) {
      const std::string record_op = std::string(path) + " " + kDotOps[j];
      const std::string label = std::string(c.label) + "." + kComponents[j];
      context.record_op = record_op.c_str();
      pass &= VshReportRounding(op.c_str(), label.c_str(), outputs[i][j],
                                c.expected[i][j], &context);
    }
  }
  return pass;
}

bool ComparePaths(const char* label, const uint32_t (*attr)[4],
                  const uint32_t (*constant)[4], uint32_t count,
                  const char* const* ops) {
  bool pass = true;
  for (uint32_t i = 0; i < count; ++i) {
    uint32_t mismatch_mask = 0;
    for (uint32_t c = 0; c < 4; ++c) {
      if (attr[i][c] != constant[i][c]) {
        mismatch_mask |= 1 << c;
      }
    }
    const LogFields fields = TestFields("label", label, "op", ops[i],
                                        "mismatch_mask", mismatch_mask);
    if (!mismatch_mask) {
      TEST_PASS_EX(fields, "%s %s: attribute and constant paths agree", ops[i],
                   label);
    } else {
      TEST_FAIL_EX(fields,
                   "%s %s: attribute and constant paths differ (mask 0x%lX)",
                   ops[i], label, static_cast<unsigned long>(mismatch_mask));
      pass = false;
    }
  }
  return pass;
}

}  // namespace

bool TestVshExceptionalAttribute::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  bool pass = true;
  for (const auto& c : kPathCases) {
    const VshConstant inputs[] = {
        {96, {c.b[0], c.b[1], c.b[2], c.b[3]}},
        {97, {c.c[0], c.c[1], c.c[2], c.c[3]}},
        {98, {c.a[0], c.a[1], c.a[2], c.a[3]}},
    };
    const ArrayAttribute v1 = {NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F, 4,
                               sizeof(c.a), c.a};

    uint32_t attr[kPathOutputs][4];
    uint32_t constant[kPathOutputs][4];
    if (!VshComputeArray(ComputeShader::kExcPathsAttr, inputs, 2, v1, 184,
                         kPathOutputs, attr) ||
        !VshCompute(ComputeShader::kExcPathsConst, inputs, std::size(inputs),
                    184, kPathOutputs, constant)) {
      TEST_FAIL("Compute draw failed.");
      return false;
    }

    pass &= ReportPathCase("attr", c, attr);
    pass &= ReportPathCase("const", c, constant);
    pass &= ComparePaths(c.label, attr, constant, kPathOutputs, kPathOps);
  }
  return pass;
}

bool TestVshExceptionalFogChain::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  static constexpr uint32_t kChainOutputs = 5;
  static constexpr const char* kChainOps[kChainOutputs] = {
      "rcp(P.w)", "P * rcp(P.w)", "d2", "rsq(d2)", "d2 * rsq(d2)"};

  bool pass = true;
  for (const auto& c : kChainCases) {
    const VshConstant input = {96, {c.p[0], c.p[1], c.p[2], c.p[3]}};
    const ArrayAttribute v1 = {NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F, 4,
                               sizeof(c.p), c.p};

    uint32_t constant[kChainOutputs][4];
    uint32_t attr[kChainOutputs][4];
    if (!VshCompute(ComputeShader::kExcFogChainConst, &input, 1, 187,
                    kChainOutputs, constant) ||
        !VshComputeArray(ComputeShader::kExcFogChainAttr, nullptr, 0, v1, 187,
                         kChainOutputs, attr)) {
      TEST_FAIL("Compute draw failed.");
      return false;
    }

    const struct {
      const char* path;
      const uint32_t (*outputs)[4];
    } paths[] = {{"const", constant}, {"attr", attr}};

    for (const auto& p : paths) {
      auto op = [&p](const char* name) {
        return std::string(p.path) + " " + name;
      };
      const VshRecordContext context = {
          "fog_chain", nullptr, nullptr, nullptr, {{"in_P", c.p, 4}}};
      pass &= VshReportRounding(op(kChainOps[0]).c_str(), c.label,
                                p.outputs[0][0], c.rcp_w, &context);
      pass &= VshReportVector(op(kChainOps[1]).c_str(), c.label, p.outputs[1],
                              c.p_over_w, context);
      pass &= VshReportRounding(op(kChainOps[2]).c_str(), c.label,
                                p.outputs[2][0], c.d2, &context);
      pass &= VshReportRounding(op(kChainOps[3]).c_str(), c.label,
                                p.outputs[3][0], c.rsq_d2, &context);
      pass &= VshReportRounding(op(kChainOps[4]).c_str(), c.label,
                                p.outputs[4][0], c.d, &context);
    }
    pass &= ComparePaths(c.label, attr, constant, kChainOutputs, kChainOps);
  }
  return pass;
}
