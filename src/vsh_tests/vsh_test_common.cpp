#include "vsh_test_common.h"

#include <pbkit/pbkit.h>
#include <windows.h>

#include <cmath>
#include <cstring>
#include <memory>

#include "nv2astate.h"
#include "pushbuffer.h"
#include "shaders/vertex_shader_program.h"
#include "test_util.h"

using PBKitPlusPlus::NV2AState;
using PBKitPlusPlus::Pushbuffer;
using PBKitPlusPlus::VertexShaderProgram;

// clang-format off
static const uint32_t kOffset0Vsh[] = {
#include "vsh_a0_relative.vshinc"
};

static const uint32_t kOffset191Vsh[] = {
#include "vsh_a0_relative_max_offset.vshinc"
};
static const uint32_t kLogVsh[] = {
#include "vsh_log.vshinc"
};
static const uint32_t kLogDenormalAttrVsh[] = {
#include "vsh_log_denormal_attr.vshinc"
};
static const uint32_t kLogDenormalConstVsh[] = {
#include "vsh_log_denormal_const.vshinc"
};
static const uint32_t kMulDenormalAttrVsh[] = {
#include "vsh_mul_denormal_attr.vshinc"
};
static const uint32_t kMulDenormalConstVsh[] = {
#include "vsh_mul_denormal_const.vshinc"
};
// clang-format on

static constexpr uint32_t kMaxProbes = 64;
static constexpr uint32_t kProbeSize = 16;
static constexpr uint32_t kProbesPerRow = kFramebufferWidth / kProbeSize;
static constexpr uint32_t kMaxTextureSize = 64;
static constexpr uint32_t kMaxTextureDepth = 1;
static constexpr uint32_t kConstantsPerPush = 4;

static std::unique_ptr<NV2AState> g_state;
static std::shared_ptr<VertexShaderProgram> g_offset0_shader;
static std::shared_ptr<VertexShaderProgram> g_offset191_shader;
static std::shared_ptr<VertexShaderProgram> g_log_shader;
static std::shared_ptr<VertexShaderProgram> g_log_denormal_attr_shader;
static std::shared_ptr<VertexShaderProgram> g_log_denormal_const_shader;
static std::shared_ptr<VertexShaderProgram> g_mul_denormal_attr_shader;
static std::shared_ptr<VertexShaderProgram> g_mul_denormal_const_shader;

static void SetupRenderState(NV2AState& state) {
  state.SetSurfaceFormat(NV2AState::SCF_A8R8G8B8, NV2AState::SZF_Z16,
                         state.GetFramebufferWidth(),
                         state.GetFramebufferHeight());

  const uint32_t pitch = state.GetFramebufferWidth() * 4;
  Pushbuffer::Begin();
  Pushbuffer::Push(NV097_SET_SURFACE_PITCH,
                   SET_MASK(NV097_SET_SURFACE_PITCH_COLOR, pitch) |
                       SET_MASK(NV097_SET_SURFACE_PITCH_ZETA, pitch));
  Pushbuffer::Push(NV097_SET_SURFACE_CLIP_HORIZONTAL,
                   state.GetFramebufferWidth() << 16);
  Pushbuffer::Push(NV097_SET_SURFACE_CLIP_VERTICAL, state.GetFramebufferHeight()
                                                        << 16);
  Pushbuffer::Push(NV097_SET_LIGHTING_ENABLE, false);
  Pushbuffer::Push(NV097_SET_SPECULAR_ENABLE, false);
  Pushbuffer::Push(NV097_SET_FRONT_POLYGON_MODE,
                   NV097_SET_FRONT_POLYGON_MODE_V_FILL);
  Pushbuffer::Push(NV097_SET_BACK_POLYGON_MODE,
                   NV097_SET_FRONT_POLYGON_MODE_V_FILL);
  Pushbuffer::Push(NV097_SET_SHADE_MODEL, NV097_SET_SHADE_MODEL_SMOOTH);
  Pushbuffer::Push(NV097_SET_FOG_ENABLE, false);
  Pushbuffer::Push(NV097_SET_CULL_FACE_ENABLE, false);
  Pushbuffer::Push(NV097_SET_COLOR_MASK,
                   NV097_SET_COLOR_MASK_BLUE_WRITE_ENABLE |
                       NV097_SET_COLOR_MASK_GREEN_WRITE_ENABLE |
                       NV097_SET_COLOR_MASK_RED_WRITE_ENABLE |
                       NV097_SET_COLOR_MASK_ALPHA_WRITE_ENABLE);
  Pushbuffer::Push(NV097_SET_DEPTH_TEST_ENABLE, false);
  Pushbuffer::Push(NV097_SET_STENCIL_TEST_ENABLE, false);
  Pushbuffer::Push(NV097_SET_NORMALIZATION_ENABLE, false);
  Pushbuffer::End();

  NV2AState::SetWindowClipExclusive(false);
  NV2AState::SetWindowClip(state.GetFramebufferWidth(),
                           state.GetFramebufferHeight());
  state.SetBlend(false);

  state.DisableTextureStages();
  state.SetupTextureStages();

  state.ClearInputColorCombiners();
  state.ClearInputAlphaCombiners();
  state.ClearOutputColorCombiners();
  state.ClearOutputAlphaCombiners();
  state.SetCombinerControl(1);
  state.SetFinalCombiner0Just(NV2AState::SRC_DIFFUSE);
  state.SetFinalCombiner1Just(NV2AState::SRC_ZERO, true, true);

  state.SetXDKDefaultViewportAndFixedFunctionMatrices();
  state.SetDepthBufferFloatMode(false);
}

static void UploadProbeConstants() {
  float constants[kVshConstantCount][4];
  for (uint32_t i = 0; i < kVshConstantCount; ++i) {
    constants[i][0] = static_cast<float>(i) / 255.0f;
    constants[i][1] = 1.0f;
    constants[i][2] = 128.0f / 255.0f;
    constants[i][3] = 1.0f;
  }

  const auto* words = reinterpret_cast<const DWORD*>(constants);
  Pushbuffer::Begin();
  Pushbuffer::Push(NV097_SET_TRANSFORM_CONSTANT_LOAD, 0);
  for (uint32_t i = 0; i < kVshConstantCount; i += kConstantsPerPush) {
    Pushbuffer::PushN(NV097_SET_TRANSFORM_CONSTANT, kConstantsPerPush * 4,
                      words + i * 4);
  }
  Pushbuffer::End();
}

bool VshInit() {
  if (g_state) {
    return true;
  }

  g_state = std::make_unique<NV2AState>(kFramebufferWidth, kFramebufferHeight,
                                        kMaxTextureSize, kMaxTextureSize,
                                        kMaxTextureDepth);
  SetupRenderState(*g_state);

  g_offset0_shader =
      std::make_shared<VertexShaderProgram>(kOffset0Vsh, sizeof(kOffset0Vsh));
  g_offset191_shader = std::make_shared<VertexShaderProgram>(
      kOffset191Vsh, sizeof(kOffset191Vsh));
  g_log_shader =
      std::make_shared<VertexShaderProgram>(kLogVsh, sizeof(kLogVsh));
  g_log_denormal_attr_shader = std::make_shared<VertexShaderProgram>(
      kLogDenormalAttrVsh, sizeof(kLogDenormalAttrVsh));
  g_log_denormal_const_shader = std::make_shared<VertexShaderProgram>(
      kLogDenormalConstVsh, sizeof(kLogDenormalConstVsh));
  g_mul_denormal_attr_shader = std::make_shared<VertexShaderProgram>(
      kMulDenormalAttrVsh, sizeof(kMulDenormalAttrVsh));
  g_mul_denormal_const_shader = std::make_shared<VertexShaderProgram>(
      kMulDenormalConstVsh, sizeof(kMulDenormalConstVsh));

  NV2AState::PBKitBusyWait();
  return true;
}

static void SelectShader(NV2AState& state, ProbeShader shader) {
  switch (shader) {
    case ProbeShader::kOffset0:
      state.SetVertexShaderProgram(g_offset0_shader);
      break;
    case ProbeShader::kOffset191:
      state.SetVertexShaderProgram(g_offset191_shader);
      break;
    case ProbeShader::kLogX:
      state.SetVertexShaderProgram(g_log_shader);
      break;
    case ProbeShader::kLogDenormalAttr:
      state.SetVertexShaderProgram(g_log_denormal_attr_shader);
      break;
    case ProbeShader::kLogDenormalConst:
      state.SetVertexShaderProgram(g_log_denormal_const_shader);
      break;
    case ProbeShader::kMulDenormalAttr:
      state.SetVertexShaderProgram(g_mul_denormal_attr_shader);
      break;
    case ProbeShader::kMulDenormalConst:
      state.SetVertexShaderProgram(g_mul_denormal_const_shader);
      break;
  }
}

static void ReadProbes(size_t count, uint32_t* out_rgb) {
  const auto* fb = reinterpret_cast<const uint8_t*>(pb_back_buffer());
  const uint32_t pitch = pb_back_buffer_pitch();
  for (size_t i = 0; i < count; ++i) {
    const uint32_t x = (i % kProbesPerRow) * kProbeSize + kProbeSize / 2;
    const uint32_t y = (i / kProbesPerRow) * kProbeSize + kProbeSize / 2;
    const auto* pixel = reinterpret_cast<const uint32_t*>(fb + y * pitch) + x;
    out_rgb[i] = *pixel & 0x00FFFFFF;
  }
}

bool VshRunRawProbes(ProbeShader shader, const uint32_t* bits, size_t count,
                     uint32_t* out_rgb, uint32_t* pgraph_intr) {
  if (!g_state || count > kMaxProbes) {
    return false;
  }

  const bool use_constant = shader == ProbeShader::kLogDenormalConst ||
                            shader == ProbeShader::kMulDenormalConst;

  auto& state = *g_state;
  SelectShader(state, shader);
  state.PrepareDraw(0xFF000000);
  UploadProbeConstants();

  const float scale_words[4] = {0x1p126f, 0x1p75f, 0x1p74f, 1.0f};
  Pushbuffer::Begin();
  Pushbuffer::Push(NV097_SET_TRANSFORM_CONSTANT_LOAD, 189);
  Pushbuffer::PushN(NV097_SET_TRANSFORM_CONSTANT, 4,
                    reinterpret_cast<const DWORD*>(scale_words));
  Pushbuffer::End();

  const float size = static_cast<float>(kProbeSize);
  for (size_t i = 0; i < count; ++i) {
    const float left = static_cast<float>((i % kProbesPerRow) * kProbeSize);
    const float top = static_cast<float>((i / kProbesPerRow) * kProbeSize);

    // The raw bits are never used in float arithmetic on the CPU.
    uint32_t words[4] = {bits[i], 0, 0, 0};
    float attr[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    if (use_constant) {
      Pushbuffer::Begin();
      Pushbuffer::Push(NV097_SET_TRANSFORM_CONSTANT_LOAD, 190);
      Pushbuffer::PushN(NV097_SET_TRANSFORM_CONSTANT, 4,
                        reinterpret_cast<const DWORD*>(words));
      Pushbuffer::End();
    } else {
      memcpy(&attr[0], &bits[i], sizeof(attr[0]));
    }

    state.Begin(NV2AState::PRIMITIVE_QUADS);
    for (uint32_t corner = 0; corner < 4; ++corner) {
      const float x = left + ((corner == 1 || corner == 2) ? size : 0.0f);
      const float y = top + ((corner >= 2) ? size : 0.0f);
      state.SetTexCoord0(attr[0], attr[1], attr[2], attr[3]);
      state.SetVertex(x, y, 0.1f);
    }
    state.End();
  }

  NV2AState::PBKitBusyWait();
  *pgraph_intr = VIDEOREG(NV_PGRAPH_INTR);
  ReadProbes(count, out_rgb);

  state.SetVertexShaderProgram(nullptr);
  return true;
}

bool VshRunProbes(ProbeShader shader, const float* a0_values, size_t count,
                  uint32_t* out_rgb, uint32_t* pgraph_intr) {
  if (!g_state || count > kMaxProbes) {
    return false;
  }

  auto& state = *g_state;
  SelectShader(state, shader);
  state.PrepareDraw(0xFF000000);
  UploadProbeConstants();

  const float size = static_cast<float>(kProbeSize);
  for (size_t i = 0; i < count; ++i) {
    const float left = static_cast<float>((i % kProbesPerRow) * kProbeSize);
    const float top = static_cast<float>((i / kProbesPerRow) * kProbeSize);

    state.Begin(NV2AState::PRIMITIVE_QUADS);
    state.SetTexCoord0(a0_values[i], 0.0f, 0.0f, 1.0f);
    state.SetVertex(left, top, 0.1f);
    state.SetTexCoord0(a0_values[i], 0.0f, 0.0f, 1.0f);
    state.SetVertex(left + size, top, 0.1f);
    state.SetTexCoord0(a0_values[i], 0.0f, 0.0f, 1.0f);
    state.SetVertex(left + size, top + size, 0.1f);
    state.SetTexCoord0(a0_values[i], 0.0f, 0.0f, 1.0f);
    state.SetVertex(left, top + size, 0.1f);
    state.End();
  }

  NV2AState::PBKitBusyWait();
  *pgraph_intr = VIDEOREG(NV_PGRAPH_INTR);

  ReadProbes(count, out_rgb);

  state.SetVertexShaderProgram(nullptr);
  return true;
}

static bool Near(uint32_t actual, uint32_t expected) {
  return actual + 1 >= expected && actual <= expected + 1;
}

A0ReadResult DecodeA0Read(uint32_t rgb) {
  const uint32_t r = (rgb >> 16) & 0xFF;
  const uint32_t g = (rgb >> 8) & 0xFF;
  const uint32_t b = rgb & 0xFF;

  if (r == 0 && g == 0 && b == 0) {
    return {A0ReadKind::kZero, 0};
  }
  if (Near(g, 255) && Near(b, 128) && r < kVshConstantCount) {
    return {A0ReadKind::kConstant, r};
  }
  return {A0ReadKind::kOther, 0};
}

static const char* KindName(A0ReadKind kind) {
  switch (kind) {
    case A0ReadKind::kConstant:
      return "constant";
    case A0ReadKind::kZero:
      return "zero";
    default:
      return "other";
  }
}

bool VshReportA0Probes(const float* a0_values, size_t count,
                       int32_t static_offset, const uint32_t* rgb,
                       uint32_t pgraph_intr) {
  bool pass = true;

  TEST_INFO_EX(TestFields("pgraph_intr", Hex32(pgraph_intr)),
               "NV_PGRAPH_INTR after draw: 0x%08X", pgraph_intr);

  for (size_t i = 0; i < count; ++i) {
    const auto a0 = static_cast<int32_t>(a0_values[i]);
    const int64_t effective = static_cast<int64_t>(a0) + static_offset;
    const bool in_range = effective >= 0 && effective < kVshConstantCount;
    const A0ReadResult result = DecodeA0Read(rgb[i]);

    const LogFields fields = TestFields(
        "a0", a0, "static_offset", static_offset, "effective_index",
        static_cast<int32_t>(effective), "rgb", Hex32(rgb[i]), "kind",
        KindName(result.kind), "decoded_index", result.index);

    if (in_range) {
      const bool ok = result.kind == A0ReadKind::kConstant &&
                      Near(result.index, static_cast<uint32_t>(effective));
      if (ok) {
        TEST_PASS_EX(fields, "A0=%d +%d -> c[%d] read back correctly", a0,
                     static_offset, static_cast<int32_t>(effective));
      } else {
        TEST_FAIL_EX(fields, "A0=%d +%d -> c[%d] read back as %s (0x%06X)", a0,
                     static_offset, static_cast<int32_t>(effective),
                     KindName(result.kind), rgb[i]);
        pass = false;
      }
      continue;
    }

    TEST_INFO_EX(fields, "A0=%d +%d (index %d, out of range) -> %s index=%u",
                 a0, static_offset, static_cast<int32_t>(effective),
                 KindName(result.kind), result.index);
  }

  return pass;
}
