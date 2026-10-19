#include "test_vsh_log_denormal.h"

#include <iterator>

#include "vsh_test_common.h"

namespace {

constexpr uint32_t kExponentBias = 160;

struct Probe {
  uint32_t bits;
  bool is_denormal;
};

constexpr Probe kProbes[] = {
    {0x00800000, false}, {0x00000000, false}, {0x007FFFFF, true},
    {0x00400000, true},  {0x00000001, true},  {0x80000001, true},
};
constexpr size_t kProbeCount = std::size(kProbes);
constexpr size_t kDenormalProbe = 4;  // 0x00000001

enum class Outcome {
  kOperandReadAsZero,
  kLogFlushes,
  kFieldExponent,
  kNormalized,
  kUnrecognized,
};

const char* OutcomeName(Outcome outcome) {
  switch (outcome) {
    case Outcome::kOperandReadAsZero:
      return "operand_read_as_zero";
    case Outcome::kLogFlushes:
      return "preserved_log_flushes";
    case Outcome::kFieldExponent:
      return "preserved_field_exponent";
    case Outcome::kNormalized:
      return "preserved_normalized";
    default:
      return "unrecognized";
  }
}

bool Near(uint32_t actual, uint32_t expected) {
  return actual + 1 >= expected && actual <= expected + 1;
}

uint32_t Red(uint32_t rgb) { return (rgb >> 16) & 0xFF; }
uint32_t Green(uint32_t rgb) { return (rgb >> 8) & 0xFF; }
uint32_t Blue(uint32_t rgb) { return rgb & 0xFF; }

int32_t TrueExponent(uint32_t bits) {
  const uint32_t magnitude = bits & 0x7FFFFFFF;
  if (magnitude >= 0x00800000) {
    return static_cast<int32_t>(magnitude >> 23) - 127;
  }
  int32_t highest_bit = 0;
  for (uint32_t m = magnitude; m > 1; m >>= 1) {
    ++highest_bit;
  }
  return highest_bit - 149;
}

uint32_t ExpectedRed(Outcome outcome, uint32_t bits) {
  switch (outcome) {
    case Outcome::kFieldExponent:
      return kExponentBias - 127;
    case Outcome::kNormalized:
      return static_cast<uint32_t>(TrueExponent(bits) + kExponentBias);
    default:
      return 0;
  }
}

bool RunPath(const char* path, ProbeShader log_shader, ProbeShader mul_shader,
             uint32_t* log_rgb, uint32_t* mul_rgb) {
  uint32_t bits[kProbeCount];
  for (size_t i = 0; i < kProbeCount; ++i) {
    bits[i] = kProbes[i].bits;
  }

  uint32_t intr = 0;
  if (!VshRunRawProbes(log_shader, bits, kProbeCount, log_rgb, &intr) ||
      !VshRunRawProbes(mul_shader, bits, kProbeCount, mul_rgb, &intr)) {
    TEST_FAIL_EX(TestFields("path", path), "Probe draw failed for %s path.",
                 path);
    return false;
  }
  return true;
}

bool CheckControls(const char* path, const uint32_t* log_rgb,
                   const uint32_t* mul_rgb) {
  bool pass = true;

  const uint32_t normal_red = Red(log_rgb[0]);
  const uint32_t normal_blue = Blue(log_rgb[0]);
  const bool normal_ok = Near(normal_red, kExponentBias - 126) &&
                         Near(normal_blue, kExponentBias - 126) &&
                         Green(log_rgb[0]) == 0;
  const LogFields normal_fields =
      TestFields("path", path, "rgb", Hex32(log_rgb[0]));
  if (normal_ok) {
    TEST_PASS_EX(normal_fields, "%s: LOGP(2^-126) == (-126, 1, -126)", path);
  } else {
    TEST_FAIL_EX(normal_fields,
                 "%s: LOGP(2^-126) rgb=0x%06X, expected 0x%02X00%02X", path,
                 log_rgb[0], kExponentBias - 126, kExponentBias - 126);
    pass = false;
  }

  const bool zero_ok = log_rgb[1] == 0;
  const LogFields zero_fields =
      TestFields("path", path, "rgb", Hex32(log_rgb[1]));
  if (zero_ok) {
    TEST_PASS_EX(zero_fields, "%s: LOGP(0) == (-inf, 1, -inf)", path);
  } else {
    TEST_FAIL_EX(zero_fields, "%s: LOGP(0) rgb=0x%06X, expected 0", path,
                 log_rgb[1]);
    pass = false;
  }

  const bool mul_normal_ok = Red(mul_rgb[0]) >= 254;
  const bool mul_zero_ok = mul_rgb[1] == 0;
  const LogFields mul_fields =
      TestFields("path", path, "normal_rgb", Hex32(mul_rgb[0]), "zero_rgb",
                 Hex32(mul_rgb[1]));
  if (mul_normal_ok && mul_zero_ok) {
    TEST_PASS_EX(mul_fields, "%s: multiply controls pass through", path);
  } else {
    TEST_FAIL_EX(
        mul_fields,
        "%s: multiply controls wrong (2^-126*2^126 rgb=0x%06X, 0*2^126 "
        "rgb=0x%06X)",
        path, mul_rgb[0], mul_rgb[1]);
    pass = false;
  }
  return pass;
}

Outcome Classify(uint32_t log_rgb, uint32_t mul_rgb) {
  const bool preserved = Red(mul_rgb) != 0 || Green(mul_rgb) != 0;
  const uint32_t red = Red(log_rgb);
  if (red == 0) {
    return preserved ? Outcome::kLogFlushes : Outcome::kOperandReadAsZero;
  }
  if (Near(red, kExponentBias - 127)) {
    return Outcome::kFieldExponent;
  }
  if (Near(red, kExponentBias - 149)) {
    return Outcome::kNormalized;
  }
  return Outcome::kUnrecognized;
}

}  // namespace

bool TestVshLogDenormal::Test() {
  if (!VshInit()) {
    TEST_FAIL("Failed to initialize vertex shader test state.");
    return false;
  }

  uint32_t attr_log[kProbeCount];
  uint32_t attr_mul[kProbeCount];
  uint32_t const_log[kProbeCount];
  uint32_t const_mul[kProbeCount];

  if (!RunPath("attribute", ProbeShader::kLogDenormalAttr,
               ProbeShader::kMulDenormalAttr, attr_log, attr_mul) ||
      !RunPath("constant", ProbeShader::kLogDenormalConst,
               ProbeShader::kMulDenormalConst, const_log, const_mul)) {
    return false;
  }

  bool pass = true;
  pass &= CheckControls("attribute", attr_log, attr_mul);
  pass &= CheckControls("constant", const_log, const_mul);

  const struct {
    const char* name;
    const uint32_t* log_rgb;
    const uint32_t* mul_rgb;
  } paths[] = {
      {"attribute", attr_log, attr_mul},
      {"constant", const_log, const_mul},
  };

  Outcome outcomes[std::size(paths)];
  for (size_t p = 0; p < std::size(paths); ++p) {
    for (size_t i = 0; i < kProbeCount; ++i) {
      const uint32_t log_rgb = paths[p].log_rgb[i];
      const uint32_t mul_rgb = paths[p].mul_rgb[i];
      TEST_INFO_EX(
          TestFields("path", paths[p].name, "bits", Hex32(kProbes[i].bits),
                     "log_rgb", Hex32(log_rgb), "exponent_red", Red(log_rgb),
                     "mantissa_green", Green(log_rgb), "log2_blue",
                     Blue(log_rgb), "mul_rgb", Hex32(mul_rgb)),
          "%s 0x%08X: LOGP rgb=0x%06X (exp=%d mant=%u log2=%d) mul rgb=0x%06X",
          paths[p].name, kProbes[i].bits, log_rgb,
          static_cast<int32_t>(Red(log_rgb)) -
              static_cast<int32_t>(kExponentBias),
          Green(log_rgb),
          static_cast<int32_t>(Blue(log_rgb)) -
              static_cast<int32_t>(kExponentBias),
          mul_rgb);
    }

    outcomes[p] = Classify(paths[p].log_rgb[kDenormalProbe],
                           paths[p].mul_rgb[kDenormalProbe]);
    TEST_INFO_EX(
        TestFields("path", paths[p].name, "outcome", OutcomeName(outcomes[p])),
        "%s path outcome: %s", paths[p].name, OutcomeName(outcomes[p]));

    if (outcomes[p] == Outcome::kUnrecognized) {
      TEST_FAIL_EX(TestFields("path", paths[p].name),
                   "%s: LOGP(2^-149) did not match any known outcome",
                   paths[p].name);
      pass = false;
      continue;
    }

    for (size_t i = 0; i < kProbeCount; ++i) {
      if (!kProbes[i].is_denormal) {
        continue;
      }
      const uint32_t expected = ExpectedRed(outcomes[p], kProbes[i].bits);
      const uint32_t actual = Red(paths[p].log_rgb[i]);
      const bool ok = Near(actual, expected);
      const LogFields fields =
          TestFields("path", paths[p].name, "bits", Hex32(kProbes[i].bits),
                     "red", actual, "expected_red", expected);
      if (ok) {
        TEST_PASS_EX(fields, "%s 0x%08X consistent with %s", paths[p].name,
                     kProbes[i].bits, OutcomeName(outcomes[p]));
      } else {
        TEST_FAIL_EX(fields, "%s 0x%08X red=%u, expected %u for %s",
                     paths[p].name, kProbes[i].bits, actual, expected,
                     OutcomeName(outcomes[p]));
        pass = false;
      }
    }
  }

  if (outcomes[0] == outcomes[1]) {
    TEST_PASS_EX(TestFields("outcome", OutcomeName(outcomes[0])),
                 "Attribute and constant paths agree: %s",
                 OutcomeName(outcomes[0]));
  } else {
    TEST_FAIL_EX(TestFields("attribute", OutcomeName(outcomes[0]), "constant",
                            OutcomeName(outcomes[1])),
                 "Attribute path (%s) and constant path (%s) disagree",
                 OutcomeName(outcomes[0]), OutcomeName(outcomes[1]));
    pass = false;
  }

  return pass;
}
