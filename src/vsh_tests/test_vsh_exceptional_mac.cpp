#include "test_vsh_exceptional_mac.h"

#include <pbkit/pbkit.h>

#include <algorithm>
#include <iterator>

#include "vsh_exceptional_common.h"

using namespace VshBits;

namespace {

constexpr uint32_t kSignBit = 0x80000000;

bool IsDenormal(uint32_t bits) {
  return (bits & 0x7F800000) == 0 && (bits & 0x007FFFFF) != 0;
}

bool IsNaN(uint32_t bits) {
  return (bits & 0x7F800000) == 0x7F800000 && (bits & 0x007FFFFF) != 0;
}

RoundingExpectation MovExpectation(uint32_t input, bool negate) {
  const uint32_t sign = negate ? kSignBit : 0;
  const uint32_t hw = input ^ sign;
  const uint32_t flush = IsDenormal(input) ? ((input & kSignBit) ^ sign) : hw;
  const uint32_t canon = IsNaN(input) ? kNaN : hw;
  return {true, hw, {{"hw", hw}, {"flush", flush}, {"canon_nan", canon}}};
}

struct MovVector {
  const char* label;
  uint32_t bits[4];
};

constexpr MovVector kMovVectors[] = {
    {"NaNs", {kNaN, kQNaN, kNegNaN, kPayloadNaN}},
    {"zeros and infs", {kPosZero, kNegZero, kPosInf, kNegInf}},
    {"range edges", {kFltMax, kMinNormal, kDenormal, kNegDenormal}},
    {"mixed", {kOneAndHalf, kNegOneAndHalf, 0x7F800001, 0xFFFFFFFF}},
};

struct MovVariant {
  const char* op;
  uint32_t source_component[4];
  bool negate;
};

// Indexed by output register, c[184 + i].
constexpr MovVariant kMovVariants[] = {
    {"MOV c", {0, 1, 2, 3}, false},      {"MOV -c", {0, 1, 2, 3}, true},
    {"MOV c.wzyx", {3, 2, 1, 0}, false}, {"MOV -c.yxwz", {1, 0, 3, 2}, true},
    {"MOV r0", {0, 1, 2, 3}, false},     {"MOV -r0", {0, 1, 2, 3}, true},
    {"MOV v1", {0, 1, 2, 3}, false},     {"MOV -v1", {0, 1, 2, 3}, true},
};
constexpr uint32_t kMovOutputs = std::size(kMovVariants);

constexpr BinaryRoundingCase kMulCases[] = {
    {"+0 * +inf", kPosZero, kPosInf, Expect(kPosZero)},
    {"+0 * -inf", kPosZero, kNegInf, Expect(kPosZero)},
    {"-0 * +inf", kNegZero, kPosInf, Expect(kPosZero)},
    {"-0 * -inf", kNegZero, kNegInf, Expect(kPosZero)},
    {"+inf * +0", kPosInf, kPosZero, Expect(kPosZero)},
    {"-inf * +0", kNegInf, kPosZero, Expect(kPosZero)},
    {"+0 * 0x7FFFFFFF", kPosZero, kNaN, Expect(kPosZero)},
    {"+0 * 0x7FC00000", kPosZero, kQNaN, Expect(kPosZero)},
    {"+0 * 0xFFC00000", kPosZero, kNegNaN, Expect(kPosZero)},
    {"-0 * 0x7FA5A5A5", kNegZero, kPayloadNaN, Expect(kPosZero)},
    {"0x7FFFFFFF * +0", kNaN, kPosZero, Expect(kPosZero)},
    {"0xFFC00000 * -0", kNegNaN, kNegZero, Expect(kPosZero)},
    {"denormal * +inf", kDenormal, kPosInf, Expect(kPosZero)},
    {"-denormal * +inf", kNegDenormal, kPosInf, Expect(kPosZero)},
    {"-0 * 1.5", kNegZero, kOneAndHalf, Expect(kPosZero)},
    {"+0 * -1.5", kPosZero, kNegOneAndHalf, Expect(kPosZero)},
    {"1.5 * +inf", kOneAndHalf, kPosInf, Expect(kPosInf)},
    {"-1.5 * +inf", kNegOneAndHalf, kPosInf, Expect(kNegInf)},
    {"+inf * +inf", kPosInf, kPosInf, Expect(kPosInf)},
    {"+inf * -inf", kPosInf, kNegInf, Expect(kNegInf)},
    {"FLT_MAX * +0", kFltMax, kPosZero, Expect(kPosZero)},
    {"2^-126 * 1.5", kMinNormal, kOneAndHalf, Expect(0x00C00000)},
    {"0x7FFFFFFF * 1.5", kNaN, kOneAndHalf, Expect(kNaN)},
    {"0x7FC00000 * 1.5", kQNaN, kOneAndHalf, Expect(kNaN)},
    {"0xFFC00000 * 1.5", kNegNaN, kOneAndHalf, Expect(kNaN)},
    {"1.5 * 0x7FA5A5A5", kOneAndHalf, kPayloadNaN, Expect(kNaN)},
    {"+inf * 0x7FC00000", kPosInf, kQNaN, Expect(kNaN)},
    {"FLT_MAX * 2", kFltMax, kTwo, Expect(kFltMax)},
    {"FLT_MAX * -2", kFltMax, kNegTwo, Expect(kNegFltMax)},
    {"2^-126 * 0.5", kMinNormal, kHalf, Expect(kPosZero)},
    {"-2^-126 * 0.5", 0x80800000, kHalf, Expect(kNegZero)},
};

constexpr BinaryRoundingCase kAddCases[] = {
    {"+inf + -inf", kPosInf, kNegInf, Expect(kNaN)},
    {"-inf + +inf", kNegInf, kPosInf, Expect(kNaN)},
    {"+inf + +inf", kPosInf, kPosInf, Expect(kPosInf)},
    {"-inf + -inf", kNegInf, kNegInf, Expect(kNegInf)},
    {"+inf + 1.5", kPosInf, kOneAndHalf, Expect(kPosInf)},
    {"FLT_MAX + -inf", kFltMax, kNegInf, Expect(kNegInf)},
    {"-inf + -0", kNegInf, kNegZero, Expect(kNegInf)},
    {"1.5 + 0x7FFFFFFF", kOneAndHalf, kNaN, Expect(kNaN)},
    {"1.5 + 0x7FC00000", kOneAndHalf, kQNaN, Expect(kNaN)},
    {"+inf + 0xFFC00000", kPosInf, kNegNaN, Expect(kNaN)},
    {"-0 + 0x7FA5A5A5", kNegZero, kPayloadNaN, Expect(kNaN)},
    {"0x7FC00000 + +0", kQNaN, kPosZero, Expect(kNaN)},
    {"+0 + -0", kPosZero, kNegZero, Expect(kPosZero)},
    {"-0 + +0", kNegZero, kPosZero, Expect(kPosZero)},
    {"-0 + -0", kNegZero, kNegZero, Expect(kPosZero)},
    {"FLT_MAX + FLT_MAX", kFltMax, kFltMax, Expect(kFltMax)},
    {"-FLT_MAX + -FLT_MAX", kNegFltMax, kNegFltMax, Expect(kNegFltMax)},
    {"denormal + denormal", kDenormal, kDenormal, Expect(kPosZero)},
    {"2^-126 + -denormal", kMinNormal, kNegDenormal, Expect(kMinNormal)},
    {"1.5 * 2^-126 - 2^-126", 0x00C00000, 0x80800000, Expect(kPosZero)},
};

constexpr TernaryCase kMadCases[] = {
    {"+0 * +inf + 1.5", kPosZero, kPosInf, kOneAndHalf, Expect(kOneAndHalf)},
    {"+0 * -inf + 1.5", kPosZero, kNegInf, kOneAndHalf, Expect(kOneAndHalf)},
    {"+0 * 0x7FC00000 + 1.5", kPosZero, kQNaN, kOneAndHalf,
     Expect(kOneAndHalf)},
    {"-0 * +inf + -0", kNegZero, kPosInf, kNegZero, Expect(kPosZero)},
    {"+0 * +inf + -0", kPosZero, kPosInf, kNegZero, Expect(kPosZero)},
    {"+inf * +0 + -inf", kPosInf, kPosZero, kNegInf, Expect(kNegInf)},
    {"1.5 * 2 + +inf", kOneAndHalf, kTwo, kPosInf, Expect(kPosInf)},
    {"1.5 * 2 + -inf", kOneAndHalf, kTwo, kNegInf, Expect(kNegInf)},
    {"1 * +inf + -inf", kOne, kPosInf, kNegInf, Expect(kNaN)},
    {"+inf * +inf + -inf", kPosInf, kPosInf, kNegInf, Expect(kNaN)},
    {"-inf * 1.5 + +inf", kNegInf, kOneAndHalf, kPosInf, Expect(kNaN)},
    {"1.5 * 2 + 0x7FA5A5A5", kOneAndHalf, kTwo, kPayloadNaN, Expect(kNaN)},
    {"+0 * +inf + 0x7FC00000", kPosZero, kPosInf, kQNaN, Expect(kNaN)},
    {"denormal * +inf + 1.5", kDenormal, kPosInf, kOneAndHalf,
     Expect(kOneAndHalf)},
    {"FLT_MAX * 2 - FLT_MAX", kFltMax, kTwo, kNegFltMax, Expect(kPosZero)},
    {"FLT_MAX * 1 + FLT_MAX", kFltMax, kOne, kFltMax, Expect(kFltMax)},
};

constexpr RoundingExpectation kTwoExpected = Expect(kTwo);
constexpr RoundingExpectation kThreeExpected = Expect(kThree);
constexpr RoundingExpectation kNaNExpected = Expect(kNaN);

constexpr ExceptionalDotCase kDotCases[] = {
    {"(0, 1, 1, 1) . (+inf, 1, 1, 1)",
     {kPosZero, kOne, kOne, kOne},
     {kPosInf, kOne, kOne, kOne},
     kTwoExpected,
     kThreeExpected,
     kThreeExpected},
    {"(0, 1, 1, 1) . (-inf, 1, 1, 1)",
     {kPosZero, kOne, kOne, kOne},
     {kNegInf, kOne, kOne, kOne},
     kTwoExpected,
     kThreeExpected,
     kThreeExpected},
    {"(-0, 1, 1, 1) . (+inf, 1, 1, 1)",
     {kNegZero, kOne, kOne, kOne},
     {kPosInf, kOne, kOne, kOne},
     kTwoExpected,
     kThreeExpected,
     kThreeExpected},
    {"(0, 1, 1, 1) . (0x7FC00000, 1, 1, 1)",
     {kPosZero, kOne, kOne, kOne},
     {kQNaN, kOne, kOne, kOne},
     kTwoExpected,
     kThreeExpected,
     kThreeExpected},
    {"(+inf, 1, 1, 1) . (0, 1, 1, 1)",
     {kPosInf, kOne, kOne, kOne},
     {kPosZero, kOne, kOne, kOne},
     kTwoExpected,
     kThreeExpected,
     kThreeExpected},
    {"(0x7FA5A5A5, 1, 1, 1) . (-0, 1, 1, 1)",
     {kPayloadNaN, kOne, kOne, kOne},
     {kNegZero, kOne, kOne, kOne},
     Expect(kTwo),
     Expect(kThree),
     Expect(kThree)},
    {"(denormal, 1, 1, 1) . (+inf, 1, 1, 1)",
     {kDenormal, kOne, kOne, kOne},
     {kPosInf, kOne, kOne, kOne},
     Expect(kTwo),
     Expect(kThree),
     Expect(kThree)},
    {"(1, 1, 1, 0) . (1, 1, 1, +inf)",
     {kOne, kOne, kOne, kPosZero},
     {kOne, kOne, kOne, kPosInf},
     Expect(kThree),
     Expect(kPosInf),
     kThreeExpected},
    {"(1, 1, 1, +inf) . (1, 1, 1, 0)",
     {kOne, kOne, kOne, kPosInf},
     {kOne, kOne, kOne, kPosZero},
     Expect(kThree),
     Expect(kThree),
     kThreeExpected},
    {"(1, 1, 1, 0x7FC00000) . (1, 1, 1, 1)",
     {kOne, kOne, kOne, kQNaN},
     {kOne, kOne, kOne, kOne},
     Expect(kThree),
     Expect(kFour),
     Expect(kNaN)},
    {"(+inf, +inf, 0, 0) . (1, -1, 0, 0)",
     {kPosInf, kPosInf, kPosZero, kPosZero},
     {kOne, kNegOne, kPosZero, kPosZero},
     kNaNExpected,
     kNaNExpected,
     kNaNExpected},
    {"(+inf, 1, 0, 0) . (1, 0, 0, -inf)",
     {kPosInf, kOne, kPosZero, kPosZero},
     {kOne, kPosZero, kPosZero, kNegInf},
     Expect(kPosInf),
     kNaNExpected,
     Expect(kPosInf)},
    {"(0, 0, 0, 0) . (+inf, -inf, 0x7FC00000, +inf)",
     {kPosZero, kPosZero, kPosZero, kPosZero},
     {kPosInf, kNegInf, kQNaN, kPosInf},
     Expect(kPosZero),
     Expect(kPosInf),
     Expect(kPosZero)},
    {"(1, 1, 1, 1) . (1, 1, 1, +inf)",
     {kOne, kOne, kOne, kOne},
     {kOne, kOne, kOne, kPosInf},
     Expect(kThree),
     Expect(kPosInf),
     Expect(kPosInf)},
    {"(-0, -0, -0, -0) . (1, 1, 1, 1)",
     {kNegZero, kNegZero, kNegZero, kNegZero},
     {kOne, kOne, kOne, kOne},
     Expect(kPosZero),
     Expect(kOne),
     Expect(kPosZero)},
};

// MIN, MAX, SLT and SGE order operands by their raw bits as sign-magnitude
// integers.
constexpr BinaryRoundingCase kMinCases[] = {
    {"MIN(0x7FFFFFFF, 1.5)", kNaN, kOneAndHalf, Expect(kOneAndHalf)},
    {"MIN(1.5, 0x7FFFFFFF)", kOneAndHalf, kNaN, Expect(kOneAndHalf)},
    {"MIN(0x7FC00000, 1.5)", kQNaN, kOneAndHalf, Expect(kOneAndHalf)},
    {"MIN(1.5, 0x7FC00000)", kOneAndHalf, kQNaN, Expect(kOneAndHalf)},
    {"MIN(1.5, 0xFFC00000)", kOneAndHalf, kNegNaN, Expect(kNegNaN)},
    {"MIN(1.5, 0x7FA5A5A5)", kOneAndHalf, kPayloadNaN, Expect(kOneAndHalf)},
    {"MIN(-inf, 0x7FC00000)", kNegInf, kQNaN, Expect(kNegInf)},
    {"MIN(0x7FC00000, -inf)", kQNaN, kNegInf, Expect(kNegInf)},
    {"MIN(+inf, -inf)", kPosInf, kNegInf, Expect(kNegInf)},
    {"MIN(-inf, +inf)", kNegInf, kPosInf, Expect(kNegInf)},
    {"MIN(+inf, FLT_MAX)", kPosInf, kFltMax, Expect(kFltMax)},
    {"MIN(-FLT_MAX, -inf)", kNegFltMax, kNegInf, Expect(kNegInf)},
    {"MIN(+0, -0)", kPosZero, kNegZero, Expect(kNegZero)},
    {"MIN(-0, +0)", kNegZero, kPosZero, Expect(kNegZero)},
    {"MIN(-denormal, +0)", kNegDenormal, kPosZero, Expect(kNegDenormal)},
};

constexpr BinaryRoundingCase kMaxCases[] = {
    {"MAX(0x7FFFFFFF, 1.5)", kNaN, kOneAndHalf, Expect(kNaN)},
    {"MAX(1.5, 0x7FFFFFFF)", kOneAndHalf, kNaN, Expect(kNaN)},
    {"MAX(0x7FC00000, 1.5)", kQNaN, kOneAndHalf, Expect(kQNaN)},
    {"MAX(1.5, 0x7FC00000)", kOneAndHalf, kQNaN, Expect(kQNaN)},
    {"MAX(1.5, 0xFFC00000)", kOneAndHalf, kNegNaN, Expect(kOneAndHalf)},
    {"MAX(1.5, 0x7FA5A5A5)", kOneAndHalf, kPayloadNaN, Expect(kPayloadNaN)},
    {"MAX(+inf, 0x7FC00000)", kPosInf, kQNaN, Expect(kQNaN)},
    {"MAX(0x7FC00000, +inf)", kQNaN, kPosInf, Expect(kQNaN)},
    {"MAX(+inf, -inf)", kPosInf, kNegInf, Expect(kPosInf)},
    {"MAX(-inf, +inf)", kNegInf, kPosInf, Expect(kPosInf)},
    {"MAX(-inf, -FLT_MAX)", kNegInf, kNegFltMax, Expect(kNegFltMax)},
    {"MAX(FLT_MAX, +inf)", kFltMax, kPosInf, Expect(kPosInf)},
    {"MAX(+0, -0)", kPosZero, kNegZero, Expect(kPosZero)},
    {"MAX(-0, +0)", kNegZero, kPosZero, Expect(kPosZero)},
    {"MAX(denormal, +0)", kDenormal, kPosZero, Expect(kDenormal)},
};

constexpr RoundingExpectation kTrue = Expect(kOne);
constexpr RoundingExpectation kFalse = Expect(kPosZero);

constexpr BinaryRoundingCase kSltCases[] = {
    {"SLT(0x7FFFFFFF, 1.5)", kNaN, kOneAndHalf, kFalse},
    {"SLT(1.5, 0x7FFFFFFF)", kOneAndHalf, kNaN, kTrue},
    {"SLT(0x7FC00000, +inf)", kQNaN, kPosInf, kFalse},
    {"SLT(-inf, 0x7FC00000)", kNegInf, kQNaN, kTrue},
    {"SLT(1.5, 0xFFC00000)", kOneAndHalf, kNegNaN, kFalse},
    {"SLT(-inf, +inf)", kNegInf, kPosInf, Expect(kOne)},
    {"SLT(+inf, -inf)", kPosInf, kNegInf, kFalse},
    {"SLT(+inf, +inf)", kPosInf, kPosInf, kFalse},
    {"SLT(-inf, -inf)", kNegInf, kNegInf, kFalse},
    {"SLT(FLT_MAX, +inf)", kFltMax, kPosInf, Expect(kOne)},
    {"SLT(-inf, -FLT_MAX)", kNegInf, kNegFltMax, Expect(kOne)},
    {"SLT(-0, +0)", kNegZero, kPosZero, kTrue},
    {"SLT(+0, -0)", kPosZero, kNegZero, kFalse},
    {"SLT(-denormal, +0)", kNegDenormal, kPosZero, kTrue},
    {"SLT(denormal, 2^-126)", kDenormal, kMinNormal, Expect(kOne)},
};

constexpr BinaryRoundingCase kSgeCases[] = {
    {"SGE(0x7FFFFFFF, 1.5)", kNaN, kOneAndHalf, kTrue},
    {"SGE(1.5, 0x7FFFFFFF)", kOneAndHalf, kNaN, kFalse},
    {"SGE(0x7FC00000, 0x7FC00000)", kQNaN, kQNaN, kTrue},
    {"SGE(+inf, 0xFFC00000)", kPosInf, kNegNaN, kTrue},
    {"SGE(0x7FA5A5A5, -inf)", kPayloadNaN, kNegInf, kTrue},
    {"SGE(+inf, +inf)", kPosInf, kPosInf, Expect(kOne)},
    {"SGE(-inf, -inf)", kNegInf, kNegInf, Expect(kOne)},
    {"SGE(-inf, +inf)", kNegInf, kPosInf, kFalse},
    {"SGE(+inf, FLT_MAX)", kPosInf, kFltMax, Expect(kOne)},
    {"SGE(-FLT_MAX, -inf)", kNegFltMax, kNegInf, Expect(kOne)},
    {"SGE(-0, +0)", kNegZero, kPosZero, kFalse},
    {"SGE(+0, -0)", kPosZero, kNegZero, Expect(kOne)},
    {"SGE(+0, denormal)", kPosZero, kDenormal, kFalse},
    {"SGE(denormal, 2^-126)", kDenormal, kMinNormal, kFalse},
};

constexpr VectorCase kDstCases[] = {
    {"(NaN, 0, 2, NaN), (NaN, +inf, NaN, 3)",
     {kNaN, kPosZero, kTwo, kNaN},
     {kNaN, kPosInf, kNaN, kThree},
     {Expect(kOne), Expect(kPosZero), Expect(kTwo), Expect(kThree)}},
    {"(+inf, +inf, +inf, +inf), (0, 0, 0, 0)",
     {kPosInf, kPosInf, kPosInf, kPosInf},
     {kPosZero, kPosZero, kPosZero, kPosZero},
     {Expect(kOne), Expect(kPosZero), Expect(kPosInf), Expect(kPosZero)}},
    {"(1, -0, 0x7FA5A5A5, 1), (1, +inf, 1, 0x7FC00000)",
     {kOne, kNegZero, kPayloadNaN, kOne},
     {kOne, kPosInf, kOne, kQNaN},
     {Expect(kOne), Expect(kPosZero), Expect(kPayloadNaN), Expect(kQNaN)}},
    {"(-inf, 0, -0, 1), (0x7FC00000, 0xFFC00000, 1, -inf)",
     {kNegInf, kPosZero, kNegZero, kOne},
     {kQNaN, kNegNaN, kOne, kNegInf},
     {Expect(kOne), Expect(kPosZero), Expect(kNegZero), Expect(kNegInf)}},
};

}  // namespace

bool TestVshExceptionalMov::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  bool pass = true;
  for (const auto& vector : kMovVectors) {
    const VshConstant input = {
        96, {vector.bits[0], vector.bits[1], vector.bits[2], vector.bits[3]}};
    const ArrayAttribute v1 = {NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F, 4,
                               sizeof(vector.bits), vector.bits};

    uint32_t outputs[kMovOutputs][4];
    if (!VshComputeArray(ComputeShader::kExcMov, &input, 1, v1, 184,
                         kMovOutputs, outputs)) {
      TEST_FAIL("Compute draw failed.");
      return false;
    }

    for (uint32_t i = 0; i < kMovOutputs; ++i) {
      const auto& variant = kMovVariants[i];
      RoundingExpectation expected[4];
      for (uint32_t c = 0; c < 4; ++c) {
        expected[c] = MovExpectation(vector.bits[variant.source_component[c]],
                                     variant.negate);
      }
      const VshRecordContext context = {
          "mov", nullptr, nullptr, nullptr, {{"in_a", vector.bits, 4}}};
      pass &= VshReportVector(variant.op, vector.label, outputs[i], expected,
                              context);
    }
  }
  return pass;
}

bool TestVshExceptionalMul::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return VshRunBinaryRoundingCases(ComputeShader::kRoundMul, "MUL", kMulCases,
                                   std::size(kMulCases), "mac");
}

bool TestVshExceptionalAdd::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return VshRunBinaryRoundingCases(ComputeShader::kRoundAdd, "ADD", kAddCases,
                                   std::size(kAddCases), "mac");
}

bool TestVshExceptionalMad::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return VshRunTernaryCases(ComputeShader::kRoundMad, "MAD", kMadCases,
                            std::size(kMadCases));
}

bool TestVshExceptionalDot::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return VshRunExceptionalDotCases(kDotCases, std::size(kDotCases));
}

bool TestVshExceptionalMin::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return VshRunBinaryRoundingCases(ComputeShader::kExcMin, "MIN", kMinCases,
                                   std::size(kMinCases), "mac");
}

bool TestVshExceptionalMax::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return VshRunBinaryRoundingCases(ComputeShader::kExcMax, "MAX", kMaxCases,
                                   std::size(kMaxCases), "mac");
}

bool TestVshExceptionalSlt::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return VshRunBinaryRoundingCases(ComputeShader::kExcSlt, "SLT", kSltCases,
                                   std::size(kSltCases), "mac");
}

bool TestVshExceptionalSge::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return VshRunBinaryRoundingCases(ComputeShader::kExcSge, "SGE", kSgeCases,
                                   std::size(kSgeCases), "mac");
}

bool TestVshExceptionalDst::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  static constexpr uint32_t kOutputs = 4;
  bool pass = true;
  for (size_t first = 0; first < std::size(kDstCases); first += kOutputs) {
    const size_t batch =
        std::min(std::size(kDstCases) - first, size_t{kOutputs});

    VshConstant inputs[kOutputs * 2] = {};
    for (uint32_t i = 0; i < std::size(inputs); ++i) {
      inputs[i].index = 96 + i;
    }
    for (size_t i = 0; i < batch; ++i) {
      for (uint32_t c = 0; c < 4; ++c) {
        inputs[i * 2].bits[c] = kDstCases[first + i].a[c];
        inputs[i * 2 + 1].bits[c] = kDstCases[first + i].b[c];
      }
    }

    uint32_t outputs[kOutputs][4];
    if (!VshCompute(ComputeShader::kRoundDst, inputs, std::size(inputs), 188,
                    kOutputs, outputs)) {
      TEST_FAIL("Compute draw failed.");
      return false;
    }

    for (size_t i = 0; i < batch; ++i) {
      const auto& c = kDstCases[first + i];
      const VshRecordContext context = {"mac",
                                        nullptr,
                                        nullptr,
                                        nullptr,
                                        {{"in_a", c.a, 4}, {"in_b", c.b, 4}}};
      pass &=
          VshReportVector("DST", c.label, outputs[i], c.expectation, context);
    }
  }
  return pass;
}
