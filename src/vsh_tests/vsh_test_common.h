#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_VSH_TEST_COMMON_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_VSH_TEST_COMMON_H

#include <cstddef>
#include <cstdint>

#include "test_log.h"

//! Number of constant registers in the NV2A vertex shader constant file.
constexpr uint32_t kVshConstantCount = 192;

//! Vertex shaders available to VshRunProbes.
enum class ProbeShader {
  //! diffuse = c[A0 + 0]
  kOffset0,
  //! diffuse = c[A0 + 191]
  kOffset191,
  //! diffuse = LOG(iTex0.x).x + c[0].z
  kLogX,
  //! Denormal inspection of LOGP(v), v read from iTex0.x. See
  //! vsh_log_denormal_attr.vsh for the output encoding.
  kLogDenormalAttr,
  //! As kLogDenormalAttr, with v read from c[190].x.
  kLogDenormalConst,
  //! Pass-through check of v * 2^126 and v * 2^149, v read from iTex0.x.
  kMulDenormalAttr,
  //! As kMulDenormalAttr, with v read from c[190].x.
  kMulDenormalConst,
};

//! The shader constant file is loaded with c[i] = (i / 255, 1, 0.5, 1) so that
//! a probe pixel identifies which constant was read: red holds the constant
//! index, green is 255 and blue is 128 for any valid constant. A pixel of
//! (0, 0, 0) means the read produced zeros.
enum class A0ReadKind {
  //! The read returned the contents of a constant register.
  kConstant,
  //! The read returned (0, 0, 0, 0).
  kZero,
  //! The read returned something that is neither a loaded constant nor zero.
  kOther,
};

struct A0ReadResult {
  A0ReadKind kind;
  //! The constant index decoded from the red channel (valid for kConstant).
  uint32_t index;
};

//! Initializes pbkit++ rendering state (once) and loads the probe constants.
bool VshInit();

//! Draws one quad per entry in `a0_values`, each with A0 set to the given
//! integer-valued float via a vertex attribute, and reads back the color
//! produced by `shader`. The pixel (0x00RRGGBB) for each probe is written to
//! `out_rgb`. At most 64 probes may be requested.
//!
//! Returns true if the draw completed. `pgraph_intr` receives the value of
//! NV_PGRAPH_INTR sampled after the draw completes.
bool VshRunProbes(ProbeShader shader, const float* a0_values, size_t count,
                  uint32_t* out_rgb, uint32_t* pgraph_intr);

//! Like VshRunProbes, but the per-probe input is a raw 32-bit pattern delivered
//! unmodified to the shader (via iTex0.x for the *Attr shaders or c[190].x for
//! the *Const shaders). Constants c[189] = (2^126, 2^75, 2^74, 1) are loaded
//! for the kMulDenormal* shaders.
bool VshRunRawProbes(ProbeShader shader, const uint32_t* bits, size_t count,
                     uint32_t* out_rgb, uint32_t* pgraph_intr);

A0ReadResult DecodeA0Read(uint32_t rgb);

//! Shaders available to the VshCompute* functions. Each writes its results to
//! constant registers (see the corresponding .vsh file) that are read back via
//! PGRAPH RDI after the draw.
enum class ComputeShader {
  //! c[188 + i] = c[96 + 2i] * c[97 + 2i] for i in [0, 3]
  kRoundMul,
  //! c[188 + i] = c[96 + 2i] + c[97 + 2i] for i in [0, 3]
  kRoundAdd,
  //! c[188 + i] = DST(c[96 + 2i], c[97 + 2i]) for i in [0, 3]
  kRoundDst,
  //! c[188 + i] = c[96 + 3i] * c[97 + 3i] + c[98 + 3i] for i in [0, 3]
  kRoundMad,
  //! c[188].k, c[189].k, c[190].k = DP3, DPH, DP4 of c[96 + 2k], c[97 + 2k]
  //! for k in [0, 3]
  kRoundDot,
  //! Rugby 2005 projection and screen-space epilogue. See
  //! vsh_compute_rugby_projection.vsh.
  kRugbyProjection,
};

//! A raw value loaded into a constant register before a compute draw.
struct VshConstant {
  uint32_t index;
  uint32_t bits[4];
};

VshConstant MakeConstant(uint32_t index, float x, float y, float z, float w);

//! A single vertex worth of data for v1, replicated to every vertex of a
//! VshComputeArray draw.
struct ArrayAttribute {
  //! NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_*
  uint32_t type;
  //! Number of components.
  uint32_t size;
  //! Size in bytes of `data`; also used as the array stride.
  uint32_t stride;
  const void* data;
};

//! Loads `inputs`, fills c[first_output, first_output + output_count) with a
//! sentinel, draws a quad with `shader`, and reads back the output registers
//! into `outputs` (x, y, z, w order). At most 5 outputs may be requested.
bool VshCompute(ComputeShader shader, const VshConstant* inputs,
                size_t input_count, uint32_t first_output, size_t output_count,
                uint32_t (*outputs)[4]);

//! As VshCompute, but the quad is drawn via NV097_DRAW_ARRAYS with v1 sourced
//! from `v1`.
bool VshComputeArray(ComputeShader shader, const VshConstant* inputs,
                     size_t input_count, const ArrayAttribute& v1,
                     uint32_t first_output, size_t output_count,
                     uint32_t (*outputs)[4]);

//! Value written to output registers before each compute draw.
constexpr uint32_t kVshComputeSentinel = 0xC0DEC0DE;

uint32_t FloatBits(float value);
float BitsFloat(uint32_t bits);

//! Expected value of a single compute output component. Checked components
//! must match `bits` exactly (+0 and -0 are distinct, as are NaN payloads).
struct ExpectedComponent {
  bool checked;
  uint32_t bits;

  static ExpectedComponent Ignore() { return {false, 0}; }
  static ExpectedComponent Bits(uint32_t bits) { return {true, bits}; }
  static ExpectedComponent Float(float value) {
    return {true, FloatBits(value)};
  }
};

//! Compares `actual` against `expected`, logging a single pass or fail event
//! labeled `label`. Returns true if every checked component matches.
bool VshCheckVector(const char* label, const uint32_t actual[4],
                    const ExpectedComponent expected[4]);

//! Logs the result of each probe, returning false if any in-range control
//! probe (effective index in [0, kVshConstantCount)) did not return the
//! expected constant. `static_offset` is the offset added to each A0 value by
//! the shader.
bool VshReportA0Probes(const float* a0_values, size_t count,
                       int32_t static_offset, const uint32_t* rgb,
                       uint32_t pgraph_intr);

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_VSH_TEST_COMMON_H
