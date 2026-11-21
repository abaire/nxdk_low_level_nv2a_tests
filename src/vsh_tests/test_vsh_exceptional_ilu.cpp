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
constexpr uint32_t kFltMaxExponent = 0x42FE0000;  // 127
constexpr uint32_t kNegTenPowNeg10 = 0xAEDBE6FF;
constexpr uint32_t kNegTwoPowNeg24 = 0xB3800000;
constexpr uint32_t kNegTwoPowNeg25 = 0xB3000000;
constexpr uint32_t kNegTwoPowNeg23 = 0xB4000000;
constexpr uint32_t kNegTwoPowNeg22 = 0xB4800000;
constexpr uint32_t kNegTwoPowNeg20 = 0xB5800000;
constexpr uint32_t kTwoPowNeg24 = 0x33800000;
constexpr uint32_t kEighth = 0x3E000000;
constexpr uint32_t kJustBelowNegOne = 0xBF800001;  // -1 - 2^-23
constexpr uint32_t kJustBelowNegTwo = 0xC0000001;  // -2 - 2^-22
constexpr uint32_t kJustPastNegTwoPowNeg24 = 0xB3800001;
constexpr uint32_t kNegOneAndHalfTwoPowNeg24 = 0xB3C00000;
constexpr uint32_t kNegOneAndHalfTwoPowNeg23 = 0xB4400000;
constexpr uint32_t kJustBelowNegHalf = 0xBF000001;  // -0.5 - 2^-24
constexpr uint32_t kSmallestDenormal = 0x00000001;
constexpr uint32_t kThreeQuarters = 0x3F400000;
constexpr uint32_t kTen = 0x41200000;
constexpr uint32_t kSqrtTwo = 0x3FB504F3;
constexpr uint32_t k64 = 0x42800000;
constexpr uint32_t k100 = 0x42C80000;
constexpr uint32_t k127Half = 0x42FF0000;
constexpr uint32_t k127_9961 = 0x42FFFE01;
constexpr uint32_t k127_9998 = 0x42FFFFE6;
constexpr uint32_t k128 = 0x43000000;
constexpr uint32_t k129 = 0x43010000;
constexpr uint32_t k200 = 0x43480000;
constexpr uint32_t kNeg126 = 0xC2FC0000;
constexpr uint32_t kNeg126Half = 0xC2FD0000;
constexpr uint32_t kNeg127_9961 = 0xC2FFFE01;
constexpr uint32_t kNeg128 = 0xC3000000;
constexpr uint32_t kNeg200 = 0xC3480000;

constexpr ScalarIluCase kRcpCases[] = {
    {"+0", kPosZero, Expect(kPosInf)},
    {"-0", kNegZero, Expect(kNegInf)},
    {"+inf", kPosInf, Expect(kPosZero)},
    {"-inf", kNegInf, Expect(kNegZero)},
    {"0x7FFFFFFF", kNaN, Expect(kNaN)},
    {"0x7FC00000", kQNaN, Expect(kNaN)},
    {"0xFFC00000", kNegNaN, Expect(kNaN)},
    {"0x7FA5A5A5", kPayloadNaN, Expect(kNaN)},
    {"1", kOne, Expect(kOne)},
    {"2", kTwo, Expect(kHalf)},
    {"-2", kNegTwo, Expect(kNegHalf)},
    {"FLT_MAX", kFltMax, Expect(kPosZero)},
    {"-FLT_MAX", kNegFltMax, Expect(kNegZero)},
    {"2^-126", kMinNormal, Expect(0x7E800000)},
    {"denormal", kDenormal, Expect(kPosInf)},
    {"-denormal", kNegDenormal, Expect(kNegInf)},
};

constexpr RoundingExpectation kRccNaN = Expect(kNaN);

constexpr ScalarIluCase kRccCases[] = {
    {"+0", kPosZero, Expect(kTwoPow64)},
    {"-0", kNegZero, Expect(kNegTwoPow64)},
    {"+inf", kPosInf, Expect(kTwoPowNeg64)},
    {"-inf", kNegInf, Expect(kNegTwoPowNeg64)},
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
    {"-0", kNegZero, Expect(kPosInf)},
    {"+inf", kPosInf, Expect(kPosZero)},
    {"-inf", kNegInf, Expect(kPosZero)},
    {"0x7FFFFFFF", kNaN, Expect(kNaN)},
    {"0x7FC00000", kQNaN, Expect(kNaN)},
    {"0xFFC00000", kNegNaN, Expect(kNaN)},
    {"0x7FA5A5A5", kPayloadNaN, Expect(kNaN)},
    {"1", kOne, Expect(kOne)},
    {"4", kFour, Expect(kHalf)},
    {"-4", kNegFour, Expect(kHalf)},
    {"0.25", kQuarter, Expect(kTwo)},
    {"2^-126", kMinNormal, Expect(0x5F000000)},
    {"2^126", 0x7E800000, Expect(0x20000000)},
    {"denormal", kDenormal, Expect(kPosInf)},
    {"-denormal", kNegDenormal, Expect(kPosInf)},
};

constexpr RoundingExpectation kOneExpected = Expect(kOne);

constexpr VectorIluCase kExppCases[] = {
    {"+0",
     kPosZero,
     {kOneExpected, Expect(kPosZero), kOneExpected, kOneExpected}},
    {"-0",
     kNegZero,
     {kOneExpected, Expect(kPosZero), kOneExpected, kOneExpected}},
    {"denormal",
     kDenormal,
     {kOneExpected, Expect(kPosZero), kOneExpected, kOneExpected}},
    {"-1.5",
     kNegOneAndHalf,
     {Expect(kQuarter), Expect(kHalf), Expect(0x3EB50600), kOneExpected}},
    {"2", kTwo, {Expect(kFour), Expect(kPosZero), Expect(kFour), kOneExpected}},
    {"-1e-10",
     kNegTenPowNeg10,
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"-2^-24",
     kNegTwoPowNeg24,
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"-2^-25",
     kNegTwoPowNeg25,
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"-2^-23",
     kNegTwoPowNeg23,
     {Expect(kHalf), Expect(0x3F7FFFFE), Expect(0x3F7FF88A), kOneExpected}},
    {"-2^-22",
     kNegTwoPowNeg22,
     {Expect(kHalf), Expect(0x3F7FFFFC), Expect(0x3F7FF88A), kOneExpected}},
    {"-2^-20",
     kNegTwoPowNeg20,
     {Expect(kHalf), Expect(0x3F7FFFF0), Expect(0x3F7FF88A), kOneExpected}},
    {"-1 - 2^-23",
     kJustBelowNegOne,
     {Expect(kQuarter), Expect(0x3F7FFFFE), Expect(0x3EFFF88A), kOneExpected}},
    {"-2 - 2^-22",
     kJustBelowNegTwo,
     {Expect(kEighth), Expect(0x3F7FFFFC), Expect(0x3E7FF88A), kOneExpected}},
    {"0xB3800001",
     kJustPastNegTwoPowNeg24,
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"-1.5 * 2^-24",
     kNegOneAndHalfTwoPowNeg24,
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"-1.5 * 2^-23",
     kNegOneAndHalfTwoPowNeg23,
     {Expect(kHalf), Expect(0x3F7FFFFC), Expect(0x3F7FF88A), kOneExpected}},
    {"-0.5 - 2^-24",
     kJustBelowNegHalf,
     {Expect(kHalf), Expect(0x3EFFFFFC), Expect(0x3F35021E), kOneExpected}},
    {"2^-24",
     kTwoPowNeg24,
     {kOneExpected, Expect(kTwoPowNeg24), kOneExpected, kOneExpected}},
    {"-15 * 2^-27",
     0xB3F00000,
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"0xB3FFFFFF",
     0xB3FFFFFF,
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"0xB4000001",
     0xB4000001,
     {Expect(kHalf), Expect(0x3F7FFFFC), Expect(0x3F7FF88A), kOneExpected}},
    {"-17 * 2^-27",
     0xB4080000,
     {Expect(kHalf), Expect(0x3F7FFFFC), Expect(0x3F7FF88A), kOneExpected}},
    {"-31 * 2^-27",
     0xB4780000,
     {Expect(kHalf), Expect(0x3F7FFFFC), Expect(0x3F7FF88A), kOneExpected}},
    {"-0.25 - 2^-25",
     0xBE800001,
     {Expect(kHalf), Expect(0x3F3FFFFE), Expect(0x3F574162), kOneExpected}},
    {"1 + 4/64 - 2^-23",
     0x3F87FFFF,
     {Expect(kTwo), Expect(0x3D7FFFE0), Expect(0x4005A724), kOneExpected}},
    {"1 + 32/64 - 2^-23",
     0x3FBFFFFF,
     {Expect(kTwo), Expect(0x3EFFFFFC), Expect(0x4035021E), kOneExpected}},
    {"1 + 32/64",
     kOneAndHalf,
     {Expect(kTwo), Expect(kHalf), Expect(0x40350600), kOneExpected}},
    {"1 + 61/64",
     0x3FFA0000,
     {Expect(kTwo), Expect(0x3F740000), Expect(0x4077D700), kOneExpected}},
    {"+inf",
     kPosInf,
     {Expect(kPosInf), Expect(kPosZero), Expect(kPosInf), kOneExpected}},
    {"-inf",
     kNegInf,
     {Expect(kPosZero), Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"0x7FFFFFFF",
     kNaN,
     {Expect(kPosInf), Expect(kNaN), Expect(kNaN), kOneExpected}},
    {"0x7FC00000",
     kQNaN,
     {Expect(kPosInf), Expect(kQNaN), Expect(kNaN), kOneExpected}},
    {"0xFFC00000",
     kNegNaN,
     {Expect(kPosInf), Expect(kNegNaN), Expect(kNaN), kOneExpected}},
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
     {Expect(kNegInf), kOneExpected, Expect(kNegInf), kOneExpected}},
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
     {Expect(kPosInf), kOneExpected, Expect(kPosInf), kOneExpected}},
    {"-inf",
     kNegInf,
     {Expect(kPosInf), kOneExpected, Expect(kPosInf), kOneExpected}},
    {"0x7FFFFFFF",
     kNaN,
     {Expect(kNaN), Expect(kNaN), Expect(kNaN), kOneExpected}},
    {"0x7FC00000",
     kQNaN,
     {Expect(kQNaN), Expect(kQNaN), Expect(kQNaN), kOneExpected}},
};

// Just below FLT_MAX: the result for 2^128, 2^+inf and FLT_MAX^1.
constexpr RoundingExpectation kNearFltMax = Expect(0x7F7FF88A);

// Inputs are (N.L, N.H, 0, power).
constexpr VectorCase kLitCases[] = {
    {"N.L 1, N.H 0, power 0",
     {kOne, kPosZero, kPosZero, kPosZero},
     {},
     {kOneExpected, kOneExpected, Expect(kOne), kOneExpected}},
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
     {kOneExpected, kOneExpected, Expect(kPosInf), kOneExpected}},
    {"N.L 1, N.H 0.5, power +inf",
     {kOne, kHalf, kPosZero, kPosInf},
     {},
     {kOneExpected, kOneExpected, Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H 2, power +inf",
     {kOne, kTwo, kPosZero, kPosInf},
     {},
     {kOneExpected, kOneExpected, kNearFltMax, kOneExpected}},
    {"N.L 0, N.H 0.5, power 1",
     {kPosZero, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"N.L -0, N.H 0.5, power 1",
     {kNegZero, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H -0.5, power 1",
     {kOne, kNegHalf, kPosZero, kOne},
     {},
     {kOneExpected, kOneExpected, Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H 0x7FC00000, power 0",
     {kOne, kQNaN, kPosZero, kPosZero},
     {},
     {kOneExpected, kOneExpected, Expect(kOne), kOneExpected}},
    {"N.L +inf, N.H 0.5, power 1",
     {kPosInf, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kPosInf), Expect(kHalf), kOneExpected}},
    {"N.L 0x7FC00000, N.H 0.5, power 1",
     {kQNaN, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kQNaN), Expect(kHalf), kOneExpected}},
    {"N.L 1, N.H denormal, power 0.5",
     {kOne, kDenormal, kPosZero, kHalf},
     {},
     {kOneExpected, kOneExpected, Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H 0x00000001, power 0.5",
     {kOne, kSmallestDenormal, kPosZero, kHalf},
     {},
     {kOneExpected, kOneExpected, Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H denormal, power 0",
     {kOne, kDenormal, kPosZero, kPosZero},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"N.L denormal, N.H 0.5, power 1",
     {kDenormal, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kDenormal), Expect(kPosZero), kOneExpected}},
    {"N.L -denormal, N.H 0.5, power 1",
     {kNegDenormal, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H -0, power 0",
     {kOne, kNegZero, kPosZero, kPosZero},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"N.L 1, N.H -0, power -1",
     {kOne, kNegZero, kPosZero, kNegOne},
     {},
     {kOneExpected, kOneExpected, Expect(kPosInf), kOneExpected}},
    {"N.L 1, N.H -denormal, power 0",
     {kOne, kNegDenormal, kPosZero, kPosZero},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"N.L 1, N.H -0.5, power 0",
     {kOne, kNegHalf, kPosZero, kPosZero},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"N.L 1, N.H denormal, power -1",
     {kOne, kDenormal, kPosZero, kNegOne},
     {},
     {kOneExpected, kOneExpected, Expect(kPosInf), kOneExpected}},
    {"N.L 1, N.H 0, power denormal",
     {kOne, kPosZero, kPosZero, kDenormal},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"N.L 1, N.H 0, power -denormal",
     {kOne, kPosZero, kPosZero, kNegDenormal},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"N.L 1, N.H -0.5, power -1",
     {kOne, kNegHalf, kPosZero, kNegOne},
     {},
     {kOneExpected, kOneExpected, Expect(kPosInf), kOneExpected}},
    {"N.L 1, N.H 0xFFC00000, power -1",
     {kOne, kNegNaN, kPosZero, kNegOne},
     {},
     {kOneExpected, kOneExpected, Expect(kNaN), kOneExpected}},
    {"N.L 0xFFC00000, N.H 0.5, power 1",
     {kNegNaN, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H 0.5, power 0x7FC00000",
     {kOne, kHalf, kPosZero, kQNaN},
     {},
     {kOneExpected, kOneExpected, Expect(kNaN), kOneExpected}},
    {"N.L 2^-126, N.H 0.5, power 1",
     {kMinNormal, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kMinNormal), Expect(kHalf), kOneExpected}},
    // N.L gate and y pass-through.
    {"N.L -1, N.H 0.5, power 1",
     {kNegOne, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"N.L -inf, N.H 0.5, power 1",
     {kNegInf, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"N.L 0.5, N.H 0.5, power 1",
     {kHalf, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kHalf), Expect(kHalf), kOneExpected}},
    {"N.L 2, N.H 0.5, power 1",
     {kTwo, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kTwo), Expect(kHalf), kOneExpected}},
    {"N.L FLT_MAX, N.H 0.5, power 1",
     {kFltMax, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kFltMax), Expect(kHalf), kOneExpected}},
    {"N.L -FLT_MAX, N.H 0.5, power 1",
     {kNegFltMax, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"N.L 0x7FFFFFFF, N.H 0.5, power 1",
     {kNaN, kHalf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kNaN), Expect(kHalf), kOneExpected}},
    {"N.L 0xFFC00000, N.H -0, power -1",
     {kNegNaN, kNegZero, kPosZero, kNegOne},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},

    // The unused s.z component.
    {"N.L 1, N.H 0.5, s.z NaN, power 1",
     {kOne, kHalf, kQNaN, kOne},
     {},
     {kOneExpected, kOneExpected, Expect(kHalf), kOneExpected}},
    {"N.L 1, N.H 0.5, s.z +inf, power 1",
     {kOne, kHalf, kPosInf, kOne},
     {},
     {kOneExpected, kOneExpected, Expect(kHalf), kOneExpected}},

    // Exponentiation of ordinary values.
    {"N.L 1, N.H 0.5, power 2",
     {kOne, kHalf, kPosZero, kTwo},
     {},
     {kOneExpected, kOneExpected, Expect(kQuarter), kOneExpected}},
    {"N.L 1, N.H 0.5, power 0.5",
     {kOne, kHalf, kPosZero, kHalf},
     {},
     {kOneExpected, kOneExpected, Expect(0x3F350600), kOneExpected}},
    {"N.L 1, N.H 0.5, power -1",
     {kOne, kHalf, kPosZero, kNegOne},
     {},
     {kOneExpected, kOneExpected, Expect(kTwo), kOneExpected}},
    {"N.L 1, N.H 2, power 3",
     {kOne, kTwo, kPosZero, kThree},
     {},
     {kOneExpected, kOneExpected, Expect(kEight), kOneExpected}},
    {"N.L 1, N.H 2, power 0.5",
     {kOne, kTwo, kPosZero, kHalf},
     {},
     {kOneExpected, kOneExpected, Expect(0x3FB50600), kOneExpected}},
    {"N.L 1, N.H 3, power 2",
     {kOne, kThree, kPosZero, kTwo},
     {},
     {kOneExpected, kOneExpected, Expect(0x41100520), kOneExpected}},
    {"N.L 1, N.H 1.5, power 2",
     {kOne, kOneAndHalf, kPosZero, kTwo},
     {},
     {kOneExpected, kOneExpected, Expect(0x40100520), kOneExpected}},
    {"N.L 1, N.H 0.75, power 10",
     {kOne, kThreeQuarters, kPosZero, kTen},
     {},
     {kOneExpected, kOneExpected, Expect(0x3D66D73A), kOneExpected}},
    {"N.L 1, N.H 1, power 2",
     {kOne, kOne, kPosZero, kTwo},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"N.L 1, N.H 1, power -1",
     {kOne, kOne, kPosZero, kNegOne},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},

    // Results at the edges of the float32 range.
    {"N.L 1, N.H 2, power 127",
     {kOne, kTwo, kPosZero, kFltMaxExponent},
     {},
     {kOneExpected, kOneExpected, Expect(0x7F000000), kOneExpected}},
    {"N.L 1, N.H 2, power 127.5",
     {kOne, kTwo, kPosZero, k127Half},
     {},
     {kOneExpected, kOneExpected, Expect(0x7F350600), kOneExpected}},
    {"N.L 1, N.H 2, power 128",
     {kOne, kTwo, kPosZero, k128},
     {},
     {kOneExpected, kOneExpected, kNearFltMax, kOneExpected}},
    {"N.L 1, N.H 2, power -126",
     {kOne, kTwo, kPosZero, kNeg126},
     {},
     {kOneExpected, kOneExpected, Expect(kMinNormal), kOneExpected}},
    {"N.L 1, N.H 2, power -126.5",
     {kOne, kTwo, kPosZero, kNeg126Half},
     {},
     {kOneExpected, kOneExpected, Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H 2, power -128",
     {kOne, kTwo, kPosZero, kNeg128},
     {},
     {kOneExpected, kOneExpected, Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H 4, power 64",
     {kOne, kFour, kPosZero, k64},
     {},
     {kOneExpected, kOneExpected, Expect(kPosInf), kOneExpected}},
    {"N.L 1, N.H 4, power 100",
     {kOne, kFour, kPosZero, k100},
     {},
     {kOneExpected, kOneExpected, Expect(kPosInf), kOneExpected}},
    {"N.L 1, N.H 0.25, power 100",
     {kOne, kQuarter, kPosZero, k100},
     {},
     {kOneExpected, kOneExpected, Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H FLT_MAX, power 1",
     {kOne, kFltMax, kPosZero, kOne},
     {},
     {kOneExpected, kOneExpected, kNearFltMax, kOneExpected}},
    {"N.L 1, N.H FLT_MAX, power 2",
     {kOne, kFltMax, kPosZero, kTwo},
     {},
     {kOneExpected, kOneExpected, Expect(kPosInf), kOneExpected}},

    // The power clamp. log2(sqrt(2)) = 0.5 keeps sqrt(2)^power below 2^64, so
    // the results show where the power is clamped.
    {"N.L 1, N.H sqrt(2), power 127",
     {kOne, kSqrtTwo, kPosZero, kFltMaxExponent},
     {},
     {kOneExpected, kOneExpected, Expect(0x5F3570F2), kOneExpected}},
    {"N.L 1, N.H sqrt(2), power 127.9961",
     {kOne, kSqrtTwo, kPosZero, k127_9961},
     {},
     {kOneExpected, kOneExpected, Expect(0x5F801EDA), kOneExpected}},
    {"N.L 1, N.H sqrt(2), power 127.9998",
     {kOne, kSqrtTwo, kPosZero, k127_9998},
     {},
     {kOneExpected, kOneExpected, Expect(0x5F8048EC), kOneExpected}},
    {"N.L 1, N.H sqrt(2), power 128",
     {kOne, kSqrtTwo, kPosZero, k128},
     {},
     {kOneExpected, kOneExpected, Expect(0x5F804BBA), kOneExpected}},
    {"N.L 1, N.H sqrt(2), power 129",
     {kOne, kSqrtTwo, kPosZero, k129},
     {},
     {kOneExpected, kOneExpected, Expect(0x5F804BBA), kOneExpected}},
    {"N.L 1, N.H sqrt(2), power 200",
     {kOne, kSqrtTwo, kPosZero, k200},
     {},
     {kOneExpected, kOneExpected, Expect(0x5F804BBA), kOneExpected}},
    {"N.L 1, N.H sqrt(2), power +inf",
     {kOne, kSqrtTwo, kPosZero, kPosInf},
     {},
     {kOneExpected, kOneExpected, Expect(0x5F804BBA), kOneExpected}},
    {"N.L 1, N.H sqrt(2), power -127.9961",
     {kOne, kSqrtTwo, kPosZero, kNeg127_9961},
     {},
     {kOneExpected, kOneExpected, Expect(0x1F7FB702), kOneExpected}},
    {"N.L 1, N.H sqrt(2), power -128",
     {kOne, kSqrtTwo, kPosZero, kNeg128},
     {},
     {kOneExpected, kOneExpected, Expect(0x1F7F6518), kOneExpected}},
    {"N.L 1, N.H sqrt(2), power -200",
     {kOne, kSqrtTwo, kPosZero, kNeg200},
     {},
     {kOneExpected, kOneExpected, Expect(0x1F7F6518), kOneExpected}},
    {"N.L 1, N.H sqrt(2), power -inf",
     {kOne, kSqrtTwo, kPosZero, kNegInf},
     {},
     {kOneExpected, kOneExpected, Expect(0x1F7F6518), kOneExpected}},

    // Infinite powers.
    {"N.L 1, N.H 0, power -inf",
     {kOne, kPosZero, kPosZero, kNegInf},
     {},
     {kOneExpected, kOneExpected, Expect(kPosInf), kOneExpected}},
    {"N.L 1, N.H 0.5, power -inf",
     {kOne, kHalf, kPosZero, kNegInf},
     {},
     {kOneExpected, kOneExpected, Expect(kPosInf), kOneExpected}},
    {"N.L 1, N.H 2, power -inf",
     {kOne, kTwo, kPosZero, kNegInf},
     {},
     {kOneExpected, kOneExpected, Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H 1, power +inf",
     {kOne, kOne, kPosZero, kPosInf},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"N.L 1, N.H 1, power -inf",
     {kOne, kOne, kPosZero, kNegInf},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},

    // Infinite and large negative N.H.
    {"N.L 1, N.H +inf, power 1",
     {kOne, kPosInf, kPosZero, kOne},
     {},
     {kOneExpected, kOneExpected, Expect(kPosInf), kOneExpected}},
    {"N.L 1, N.H +inf, power 0",
     {kOne, kPosInf, kPosZero, kPosZero},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"N.L 1, N.H +inf, power -1",
     {kOne, kPosInf, kPosZero, kNegOne},
     {},
     {kOneExpected, kOneExpected, Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H +inf, power 0.5",
     {kOne, kPosInf, kPosZero, kHalf},
     {},
     {kOneExpected, kOneExpected, Expect(kPosInf), kOneExpected}},
    {"N.L 1, N.H -inf, power 1",
     {kOne, kNegInf, kPosZero, kOne},
     {},
     {kOneExpected, kOneExpected, Expect(kPosZero), kOneExpected}},
    {"N.L 1, N.H -inf, power 0",
     {kOne, kNegInf, kPosZero, kPosZero},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"N.L 1, N.H -inf, power -1",
     {kOne, kNegInf, kPosZero, kNegOne},
     {},
     {kOneExpected, kOneExpected, Expect(kPosInf), kOneExpected}},
    {"N.L 1, N.H -FLT_MAX, power -1",
     {kOne, kNegFltMax, kPosZero, kNegOne},
     {},
     {kOneExpected, kOneExpected, Expect(kPosInf), kOneExpected}},
    {"N.L 1, N.H -2, power 2",
     {kOne, kNegTwo, kPosZero, kTwo},
     {},
     {kOneExpected, kOneExpected, Expect(kPosZero), kOneExpected}},

    // NaN N.H.
    {"N.L 1, N.H 0x7FC00000, power 1",
     {kOne, kQNaN, kPosZero, kOne},
     {},
     {kOneExpected, kOneExpected, Expect(kNaN), kOneExpected}},
    {"N.L 1, N.H 0x7FC00000, power -1",
     {kOne, kQNaN, kPosZero, kNegOne},
     {},
     {kOneExpected, kOneExpected, Expect(kNaN), kOneExpected}},
    {"N.L 1, N.H 0x7FC00000, power denormal",
     {kOne, kQNaN, kPosZero, kDenormal},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"N.L 1, N.H 0x7FFFFFFF, power 0",
     {kOne, kNaN, kPosZero, kPosZero},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"N.L 1, N.H 0x7FA5A5A5, power 1",
     {kOne, kPayloadNaN, kPosZero, kOne},
     {},
     {kOneExpected, kOneExpected, Expect(kNaN), kOneExpected}},
    {"N.L 1, N.H 0xFFC00000, power 0",
     {kOne, kNegNaN, kPosZero, kPosZero},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},
    {"N.L 1, N.H 0xFFC00000, power 1",
     {kOne, kNegNaN, kPosZero, kOne},
     {},
     {kOneExpected, kOneExpected, Expect(kNaN), kOneExpected}},
    {"N.L 1, N.H 0xFFC00000, power -denormal",
     {kOne, kNegNaN, kPosZero, kNegDenormal},
     {},
     {kOneExpected, kOneExpected, kOneExpected, kOneExpected}},

    // NaN power.
    {"N.L 1, N.H 0.5, power 0xFFC00000",
     {kOne, kHalf, kPosZero, kNegNaN},
     {},
     {kOneExpected, kOneExpected, Expect(kNaN), kOneExpected}},
    {"N.L 1, N.H 0.5, power 0x7FA5A5A5",
     {kOne, kHalf, kPosZero, kPayloadNaN},
     {},
     {kOneExpected, kOneExpected, Expect(kNaN), kOneExpected}},
    {"N.L 1, N.H 0, power 0x7FC00000",
     {kOne, kPosZero, kPosZero, kQNaN},
     {},
     {kOneExpected, kOneExpected, Expect(kNaN), kOneExpected}},
    {"N.L 1, N.H 1, power 0x7FC00000",
     {kOne, kOne, kPosZero, kQNaN},
     {},
     {kOneExpected, kOneExpected, Expect(kNaN), kOneExpected}},
    {"N.L 1, N.H -0.5, power 0x7FC00000",
     {kOne, kNegHalf, kPosZero, kQNaN},
     {},
     {kOneExpected, kOneExpected, Expect(kNaN), kOneExpected}},
    {"N.L 1, N.H +inf, power 0x7FC00000",
     {kOne, kPosInf, kPosZero, kQNaN},
     {},
     {kOneExpected, kOneExpected, Expect(kNaN), kOneExpected}},
    {"N.L 1, N.H 0x7FC00000, power 0x7FC00000",
     {kOne, kQNaN, kPosZero, kQNaN},
     {},
     {kOneExpected, kOneExpected, Expect(kNaN), kOneExpected}},

    // Whether the N.L gate takes precedence over NaN and infinite results.
    {"N.L 0, N.H 0.5, power 0x7FC00000",
     {kPosZero, kHalf, kPosZero, kQNaN},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"N.L -0, N.H 0.5, power 0x7FC00000",
     {kNegZero, kHalf, kPosZero, kQNaN},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"N.L -1, N.H 0.5, power 0x7FC00000",
     {kNegOne, kHalf, kPosZero, kQNaN},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"N.L denormal, N.H 0.5, power 0x7FC00000",
     {kDenormal, kHalf, kPosZero, kQNaN},
     {},
     {kOneExpected, Expect(kDenormal), Expect(kPosZero), kOneExpected}},
    {"N.L 0xFFC00000, N.H 0.5, power 0x7FC00000",
     {kNegNaN, kHalf, kPosZero, kQNaN},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"N.L 0, N.H 0x7FC00000, power 1",
     {kPosZero, kQNaN, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"N.L -1, N.H 0x7FC00000, power 1",
     {kNegOne, kQNaN, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"N.L denormal, N.H 0x7FC00000, power 1",
     {kDenormal, kQNaN, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kDenormal), Expect(kPosZero), kOneExpected}},
    {"N.L 0, N.H 0, power -1",
     {kPosZero, kPosZero, kPosZero, kNegOne},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"N.L -1, N.H 0, power -1",
     {kNegOne, kPosZero, kPosZero, kNegOne},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
    {"N.L 0, N.H +inf, power 1",
     {kPosZero, kPosInf, kPosZero, kOne},
     {},
     {kOneExpected, Expect(kPosZero), Expect(kPosZero), kOneExpected}},
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
