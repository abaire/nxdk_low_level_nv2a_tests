#include "test_vsh_exceptional_ilu.h"

#include <algorithm>
#include <iterator>

#include "vsh_exceptional_common.h"

using namespace VshBits;

namespace {

constexpr uint32_t kTwoPow64 = 0x5F800000;
constexpr uint32_t kNegTwoPow64 = 0xDF800000;
constexpr uint32_t kTwoPowNeg64 = 0x1F800000;
constexpr uint32_t kNegTwoPowNeg64 = 0x9F800000;
constexpr uint32_t kQuarter = 0x3E800000;
constexpr uint32_t kEight = 0x41000000;
constexpr uint32_t kNegEight = 0xC1000000;
constexpr uint32_t kFltMaxExponent = 0x42FE0000;    // 127
constexpr uint32_t kInfExponentField = 0x43000000;  // 128

constexpr ScalarIluCase kRcpCases[] = {
    {"+0", kPosZero, Expect(kPosInf)},
    {"-0", kNegZero, Expect(kNegInf, "pos_inf", kPosInf)},
    {"+inf", kPosInf, Expect(kPosZero)},
    {"-inf", kNegInf, Expect(kNegZero, "pos_zero", kPosZero)},
    {"0x7FFFFFFF", kNaN, Expect(kNaN)},
    {"0x7FC00000", kQNaN, Expect(kNaN, "ieee", kQNaN)},
    {"0xFFC00000", kNegNaN, Expect(kNaN, "ieee", kNegNaN)},
    {"0x7FA5A5A5", kPayloadNaN, Expect(kNaN, "ieee", kPayloadNaNQuiet)},
    {"1", kOne, Expect(kOne)},
    {"2", kTwo, Expect(kHalf)},
    {"-2", kNegTwo, Expect(kNegHalf)},
    {"FLT_MAX", kFltMax, Expect(kPosZero, "ieee", 0x00200000)},
    {"-FLT_MAX", kNegFltMax,
     Expect(kNegZero, "pos_zero", kPosZero, "ieee", 0x80200000)},
    {"2^-126", kMinNormal, Expect(0x7E800000)},
    {"denormal", kDenormal, Expect(kPosInf, "ieee", 0x7F000000)},
    {"-denormal", kNegDenormal,
     Expect(kNegInf, "pos_inf", kPosInf, "ieee", 0xFF000000)},
};

constexpr RoundingExpectation kRccNaN =
    Expect(kNaN, "clamp_max", kTwoPow64, "ieee", kQNaN);

constexpr ScalarIluCase kRccCases[] = {
    {"+0", kPosZero, Expect(kTwoPow64)},
    {"-0", kNegZero, Expect(kNegTwoPow64, "pos", kTwoPow64)},
    {"+inf", kPosInf, Expect(kTwoPowNeg64)},
    {"-inf", kNegInf, Expect(kNegTwoPowNeg64, "pos", kTwoPowNeg64)},
    {"0x7FFFFFFF", kNaN, kRccNaN},
    {"0x7FC00000", kQNaN, kRccNaN},
    {"0xFFC00000", kNegNaN, kRccNaN},
    {"1", kOne, Expect(kOne)},
    {"2", kTwo, Expect(kHalf)},
    {"-2", kNegTwo, Expect(kNegHalf)},
    {"2^-70", 0x1C800000, Expect(kTwoPow64)},
    {"-2^-70", 0x9C800000, Expect(kNegTwoPow64)},
    {"2^70", 0x62800000, Expect(kTwoPowNeg64)},
    {"FLT_MAX", kFltMax, Expect(kTwoPowNeg64)},
    {"2^-126", kMinNormal, Expect(kTwoPow64)},
    {"denormal", kDenormal, Expect(kTwoPow64)},
};

constexpr ScalarIluCase kRsqCases[] = {
    {"+0", kPosZero, Expect(kPosInf)},
    {"-0", kNegZero, Expect(kPosInf, "ieee", kNegInf)},
    {"+inf", kPosInf, Expect(kPosZero)},
    {"-inf", kNegInf, Expect(kPosZero, "ieee", kQNaN)},
    {"0x7FFFFFFF", kNaN, Expect(kNaN)},
    {"0x7FC00000", kQNaN, Expect(kNaN, "ieee", kQNaN)},
    {"0xFFC00000", kNegNaN, Expect(kNaN, "ieee", kNegNaN)},
    {"0x7FA5A5A5", kPayloadNaN, Expect(kNaN, "ieee", kPayloadNaNQuiet)},
    {"1", kOne, Expect(kOne)},
    {"4", kFour, Expect(kHalf)},
    {"-4", kNegFour, Expect(kHalf, "ieee", kQNaN)},
    {"0.25", kQuarter, Expect(kTwo)},
    {"2^-126", kMinNormal, Expect(0x5F000000)},
    {"2^126", 0x7E800000, Expect(0x20000000)},
    {"denormal", kDenormal, Expect(kPosInf, "ieee", 0x5F3504F3)},
    {"-denormal", kNegDenormal, Expect(kPosInf, "ieee", kQNaN)},
};

constexpr RoundingExpectation kOneExpected = Expect(kOne);

constexpr VectorIluCase kExppCases[] = {
    {"+0",
     kPosZero,
     {kOneExpected, Expect(kPosZero), kOneExpected, kOneExpected}},
    {"-0",
     kNegZero,
     {kOneExpected, Expect(kPosZero, "neg_zero", kNegZero), kOneExpected,
      kOneExpected}},
    {"denormal",
     kDenormal,
     {kOneExpected, Expect(kPosZero, "ieee", kDenormal), kOneExpected,
      kOneExpected}},
    {"-1.5",
     kNegOneAndHalf,
     {Expect(kQuarter), Expect(kHalf), Expect(0x3EB50600), kOneExpected}},
    {"2", kTwo, {Expect(kFour), Expect(kPosZero), Expect(kFour), kOneExpected}},
    {"+inf",
     kPosInf,
     {Expect(kPosInf), Expect(kPosZero, "ieee", kQNaN), Expect(kPosInf),
      kOneExpected}},
    {"-inf",
     kNegInf,
     {Expect(kPosZero), Expect(kPosZero, "ieee", kQNaN), Expect(kPosZero),
      kOneExpected}},
    {"0x7FFFFFFF",
     kNaN,
     {Expect(kPosInf, "canon_nan", kNaN), Expect(kNaN), Expect(kNaN),
      kOneExpected}},
    {"0x7FC00000",
     kQNaN,
     {Expect(kPosInf, "canon_nan", kNaN), Expect(kQNaN, "canon_nan", kNaN),
      Expect(kNaN, "ieee", kQNaN), kOneExpected}},
    {"0xFFC00000",
     kNegNaN,
     {Expect(kPosInf, "canon_nan", kNaN), Expect(kNegNaN, "canon_nan", kNaN),
      Expect(kNaN, "ieee", kNegNaN), kOneExpected}},
};

constexpr VectorIluCase kLogpCases[] = {
    {"+0",
     kPosZero,
     {Expect(kNegInf), kOneExpected, Expect(kNegInf), kOneExpected}},
    {"-0",
     kNegZero,
     {Expect(kNegInf), kOneExpected, Expect(kNegInf), kOneExpected}},
    {"denormal",
     kDenormal,
     {Expect(kNegInf, "ieee", 0xC2FE0000), kOneExpected,
      Expect(kNegInf, "ieee", 0xC2FE0000), kOneExpected}},
    {"1",
     kOne,
     {Expect(kPosZero), kOneExpected, Expect(kPosZero), kOneExpected}},
    {"8", kEight, {Expect(kThree), kOneExpected, Expect(kThree), kOneExpected}},
    {"-8",
     kNegEight,
     {Expect(kThree), kOneExpected, Expect(kThree), kOneExpected}},
    {"1.5",
     kOneAndHalf,
     {Expect(kPosZero), Expect(kOneAndHalf), Expect(0x3F15C780), kOneExpected}},
    {"FLT_MAX",
     kFltMax,
     {Expect(kFltMaxExponent), Expect(0x3FFFFFFF), Expect(0x42FFFFF3),
      kOneExpected}},
    {"+inf",
     kPosInf,
     {Expect(kPosInf, "bitfield", kInfExponentField), kOneExpected,
      Expect(kPosInf), kOneExpected}},
    {"-inf",
     kNegInf,
     {Expect(kPosInf, "bitfield", kInfExponentField), kOneExpected,
      Expect(kPosInf), kOneExpected}},
    {"0x7FFFFFFF",
     kNaN,
     {Expect(kNaN, "bitfield", kInfExponentField),
      Expect(kNaN, "bitfield", 0x3FFFFFFF), Expect(kNaN), kOneExpected}},
    {"0x7FC00000",
     kQNaN,
     {Expect(kQNaN, "canon_nan", kNaN, "bitfield", kInfExponentField),
      Expect(kQNaN, "canon_nan", kNaN, "bitfield", kOneAndHalf),
      Expect(kQNaN, "canon_nan", kNaN), kOneExpected}},
};

// 2^127.99...: the power is clamped before exponentiation.
constexpr RoundingExpectation kClampedPower =
    Expect(0x7F7FF88A, "no_clamp", kPosInf);

// Inputs are (N.L, N.H, 0, power).
constexpr VectorCase kLitCases[] = {
    {"N.L 1, N.H 0, power 0",
     {kOne, kPosZero, kPosZero, kPosZero},
     {},
     {kOneExpected, kOneExpected, Expect(kOne, "d3d", kPosZero, "ieee", kQNaN),
      kOneExpected}},
    {"N.L 1, N.H 0.5, power 0",
     {kOne, kHalf, kPosZero, kPosZero},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"N.L 1, N.H 0, power 2",
     {kOne, kPosZero, kPosZero, kTwo},
     {},
     {kOneExpected, kOneExpected, Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H 0, power +inf",
     {kOne, kPosZero, kPosZero, kPosInf},
     {},
     {kOneExpected, kOneExpected, Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H 0, power -1",
     {kOne, kPosZero, kPosZero, kNegOne},
     {},
     {kOneExpected, kOneExpected, Expect(kPosInf, "d3d", kPosZero),
      kOneExpected}},
    {"N.L 1, N.H 0.5, power +inf",
     {kOne, kHalf, kPosZero, kPosInf},
     {},
     {kOneExpected, kOneExpected, Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H 2, power +inf",
     {kOne, kTwo, kPosZero, kPosInf},
     {},
     {kOneExpected, kOneExpected, kClampedPower, kOneExpected}},
    {"N.L 0, N.H 0.5, power 1",
     {kPosZero, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero, "ungated", kHalf),
      kOneExpected}},
    {"N.L -0, N.H 0.5, power 1",
     {kNegZero, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kPosZero, "neg_zero", kNegZero),
      Expect(kPosZero, "ungated", kHalf), kOneExpected}},
    {"N.L 1, N.H -0.5, power 1",
     {kOne, kNegHalf, kPosZero, kOne},
     {},
     {kOneExpected, kOneExpected, Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H 0x7FC00000, power 0",
     {kOne, kQNaN, kPosZero, kPosZero},
     {},
     {kOneExpected, kOneExpected, Expect(kOne, "d3d", kPosZero, "ieee", kQNaN),
      kOneExpected}},
    {"N.L +inf, N.H 0.5, power 1",
     {kPosInf, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kPosInf), Expect(kHalf), kOneExpected}},
    {"N.L 0x7FC00000, N.H 0.5, power 1",
     {kQNaN, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kQNaN, "clamped", kPosZero),
      Expect(kHalf, "gated", kPosZero), kOneExpected}},
};

struct ArlProbe {
  const char* label;
  uint32_t bits;
  A0ReadKind expected_kind;
  //! Checked only for A0ReadKind::kConstant.
  uint32_t expected_index;
};

constexpr ArlProbe kArlProbes[] = {
    {"1", kOne, A0ReadKind::kConstant, 1},
    {"1.5", kOneAndHalf, A0ReadKind::kConstant, 1},
    {"5", 0x40A00000, A0ReadKind::kConstant, 5},
    {"+0", kPosZero, A0ReadKind::kConstant, 0},
    {"-0", kNegZero, A0ReadKind::kConstant, 0},
    {"denormal", kDenormal, A0ReadKind::kConstant, 0},
    {"-denormal", kNegDenormal, A0ReadKind::kConstant, 0},
    {"+inf", kPosInf, A0ReadKind::kZero, 0},
    {"-inf", kNegInf, A0ReadKind::kZero, 0},
    {"0x7FFFFFFF", kNaN, A0ReadKind::kZero, 0},
    {"0x7FC00000", kQNaN, A0ReadKind::kZero, 0},
    {"0xFFC00000", kNegNaN, A0ReadKind::kZero, 0},
    {"FLT_MAX", kFltMax, A0ReadKind::kZero, 0},
    {"-FLT_MAX", kNegFltMax, A0ReadKind::kZero, 0},
};

const char* A0KindName(A0ReadKind kind) {
  switch (kind) {
    case A0ReadKind::kConstant:
      return "constant";
    case A0ReadKind::kZero:
      return "zero";
    default:
      return "other";
  }
}

}  // namespace

bool TestVshExceptionalRcp::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return VshRunScalarIluCases(ComputeShader::kExcRcp, "RCP", kRcpCases,
                              std::size(kRcpCases));
}

bool TestVshExceptionalRcc::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return VshRunScalarIluCases(ComputeShader::kExcRcc, "RCC", kRccCases,
                              std::size(kRccCases));
}

bool TestVshExceptionalRsq::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return VshRunScalarIluCases(ComputeShader::kExcRsq, "RSQ", kRsqCases,
                              std::size(kRsqCases));
}

bool TestVshExceptionalExpp::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return VshRunVectorIluCases(ComputeShader::kExcExpp, "EXPP", kExppCases,
                              std::size(kExppCases));
}

bool TestVshExceptionalLogp::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }
  return VshRunVectorIluCases(ComputeShader::kExcLogp, "LOGP", kLogpCases,
                              std::size(kLogpCases));
}

bool TestVshExceptionalLit::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  static constexpr uint32_t kOutputs = 4;
  bool pass = true;
  for (size_t first = 0; first < std::size(kLitCases); first += kOutputs) {
    const size_t batch =
        std::min(std::size(kLitCases) - first, size_t{kOutputs});

    VshConstant inputs[kOutputs] = {};
    for (uint32_t i = 0; i < kOutputs; ++i) {
      inputs[i].index = 96 + i;
    }
    for (size_t i = 0; i < batch; ++i) {
      for (uint32_t c = 0; c < 4; ++c) {
        inputs[i].bits[c] = kLitCases[first + i].a[c];
      }
    }

    uint32_t outputs[kOutputs][4];
    if (!VshCompute(ComputeShader::kExcLit, inputs, kOutputs, 188, kOutputs,
                    outputs)) {
      TEST_FAIL("Compute draw failed.");
      return false;
    }

    for (size_t i = 0; i < batch; ++i) {
      const auto& c = kLitCases[first + i];
      const VshRecordContext context = {
          "ilu", nullptr, nullptr, nullptr, {{"in_a", c.a, 4}}};
      pass &=
          VshReportVector("LIT", c.label, outputs[i], c.expectation, context);
    }
  }
  return pass;
}

bool TestVshExceptionalArl::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  static constexpr size_t kCount = std::size(kArlProbes);
  uint32_t bits[kCount];
  for (size_t i = 0; i < kCount; ++i) {
    bits[i] = kArlProbes[i].bits;
  }

  uint32_t rgb[kCount];
  uint32_t pgraph_intr = 0;
  if (!VshRunRawProbes(ProbeShader::kOffset0, bits, kCount, rgb,
                       &pgraph_intr)) {
    TEST_FAIL("Probe draw failed.");
    return false;
  }

  TEST_INFO_EX(TestFields("pgraph_intr", Hex32(pgraph_intr)),
               "NV_PGRAPH_INTR after draw: 0x%08X", pgraph_intr);

  bool pass = true;
  for (size_t i = 0; i < kCount; ++i) {
    const auto& probe = kArlProbes[i];
    const A0ReadResult result = DecodeA0Read(rgb[i]);
    const LogFields fields =
        TestFields("label", probe.label, "bits", Hex32(probe.bits), "rgb",
                   Hex32(rgb[i]), "kind", A0KindName(result.kind),
                   "decoded_index", result.index, "group", "ilu", "record_op",
                   "ARL", "case", probe.label, "in_a", Hex32(probe.bits));

    const bool expect_constant = probe.expected_kind == A0ReadKind::kConstant;
    if (result.kind == probe.expected_kind &&
        (!expect_constant || result.index == probe.expected_index)) {
      if (expect_constant) {
        TEST_PASS_EX(fields, "ARL %s -> c[%u]", probe.label,
                     probe.expected_index);
      } else {
        TEST_PASS_EX(fields, "ARL %s -> out of range (zero)", probe.label);
      }
    } else if (expect_constant) {
      TEST_FAIL_EX(fields, "ARL %s -> %s index=%u rgb=0x%06lX, expected c[%u]",
                   probe.label, A0KindName(result.kind), result.index,
                   static_cast<unsigned long>(rgb[i]), probe.expected_index);
      pass = false;
    } else {
      TEST_FAIL_EX(fields,
                   "ARL %s -> %s index=%u rgb=0x%06lX, expected out of range",
                   probe.label, A0KindName(result.kind), result.index,
                   static_cast<unsigned long>(rgb[i]));
      pass = false;
    }
  }
  return pass;
}
