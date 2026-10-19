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

//! Logs the result of each probe, returning false if any in-range control
//! probe (effective index in [0, kVshConstantCount)) did not return the
//! expected constant. `static_offset` is the offset added to each A0 value by
//! the shader.
bool VshReportA0Probes(const float* a0_values, size_t count,
                       int32_t static_offset, const uint32_t* rgb,
                       uint32_t pgraph_intr);

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_VSH_TEST_COMMON_H
