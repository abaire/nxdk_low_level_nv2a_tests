#include "vsh_test_common.h"

#include <pbkit/pbkit.h>
#include <windows.h>

#include <cmath>
#include <cstring>
#include <functional>
#include <iterator>
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
static const uint32_t kComputeMacMovVsh[] = {
#include "vsh_compute_mac_mov.vshinc"
};
static const uint32_t kComputeMacAddVsh[] = {
#include "vsh_compute_mac_add.vshinc"
};
static const uint32_t kComputeIluRcpVsh[] = {
#include "vsh_compute_ilu_rcp.vshinc"
};
static const uint32_t kComputePairedIluVsh[] = {
#include "vsh_compute_paired_ilu.vshinc"
};
static const uint32_t kComputeAmericasArmyVsh[] = {
#include "vsh_compute_americas_army.vshinc"
};
static const uint32_t kComputeSpyVsSpyMenuVsh[] = {
#include "vsh_compute_spy_vs_spy_menu.vshinc"
};
static const uint32_t kComputeV1PassthroughVsh[] = {
#include "vsh_compute_v1_passthrough.vshinc"
};
static const uint32_t kComputeInlineAttributesVsh[] = {
#include "vsh_compute_inline_attributes.vshinc"
};
// clang-format on

// Indexed by ComputeShader.
struct ComputeShaderSource {
  const uint32_t* code;
  uint32_t size;
};

static const ComputeShaderSource kComputeShaders[] = {
    {kComputeMacMovVsh, sizeof(kComputeMacMovVsh)},
    {kComputeMacAddVsh, sizeof(kComputeMacAddVsh)},
    {kComputeIluRcpVsh, sizeof(kComputeIluRcpVsh)},
    {kComputePairedIluVsh, sizeof(kComputePairedIluVsh)},
    {kComputeAmericasArmyVsh, sizeof(kComputeAmericasArmyVsh)},
    {kComputeSpyVsSpyMenuVsh, sizeof(kComputeSpyVsSpyMenuVsh)},
    {kComputeV1PassthroughVsh, sizeof(kComputeV1PassthroughVsh)},
    {kComputeInlineAttributesVsh, sizeof(kComputeInlineAttributesVsh)},
};

static constexpr uint32_t kMaxProbes = 64;
static constexpr uint32_t kProbeSize = 16;
static constexpr uint32_t kProbesPerRow = kFramebufferWidth / kProbeSize;
static constexpr uint32_t kMaxTextureSize = 64;
static constexpr uint32_t kMaxTextureDepth = 1;
static constexpr uint32_t kConstantsPerPush = 4;
static constexpr uint32_t kMaxComputeOutputs = 5;
static constexpr uint32_t kComputeQuadVertices = 4;
static constexpr uint32_t kMaxArrayStride = 16;
static constexpr uint32_t kVertexAttributeCount = 16;
static constexpr uint32_t kVshConstantsRdiBase = 0x170000;

static std::unique_ptr<NV2AState> g_state;
static std::shared_ptr<VertexShaderProgram> g_offset0_shader;
static std::shared_ptr<VertexShaderProgram> g_offset191_shader;
static std::shared_ptr<VertexShaderProgram> g_log_shader;
static std::shared_ptr<VertexShaderProgram> g_log_denormal_attr_shader;
static std::shared_ptr<VertexShaderProgram> g_log_denormal_const_shader;
static std::shared_ptr<VertexShaderProgram> g_mul_denormal_attr_shader;
static std::shared_ptr<VertexShaderProgram> g_mul_denormal_const_shader;
static std::shared_ptr<VertexShaderProgram>
    g_compute_shaders[std::size(kComputeShaders)];
static float* g_array_positions;
static uint8_t* g_array_v1;

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
  for (size_t i = 0; i < std::size(kComputeShaders); ++i) {
    g_compute_shaders[i] = std::make_shared<VertexShaderProgram>(
        kComputeShaders[i].code, kComputeShaders[i].size);
  }

  g_array_positions = static_cast<float*>(MmAllocateContiguousMemoryEx(
      kComputeQuadVertices * 4 * sizeof(float), 0, MAXRAM, 0,
      PAGE_WRITECOMBINE | PAGE_READWRITE));
  g_array_v1 = static_cast<uint8_t*>(MmAllocateContiguousMemoryEx(
      kComputeQuadVertices * kMaxArrayStride, 0, MAXRAM, 0,
      PAGE_WRITECOMBINE | PAGE_READWRITE));
  if (!g_array_positions || !g_array_v1) {
    return false;
  }
  static constexpr float kArrayPositions[kComputeQuadVertices][4] = {
      {0.0f, 0.0f, 1.0f, 1.0f},
      {10.0f, 0.0f, 1.0f, 1.0f},
      {10.0f, 10.0f, 1.0f, 1.0f},
      {0.0f, 10.0f, 1.0f, 1.0f},
  };
  memcpy(g_array_positions, kArrayPositions, sizeof(kArrayPositions));

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

uint32_t FloatBits(float value) {
  uint32_t bits;
  memcpy(&bits, &value, sizeof(bits));
  return bits;
}

float BitsFloat(uint32_t bits) {
  float value;
  memcpy(&value, &bits, sizeof(value));
  return value;
}

VshConstant MakeConstant(uint32_t index, float x, float y, float z, float w) {
  return {index, {FloatBits(x), FloatBits(y), FloatBits(z), FloatBits(w)}};
}

static void LoadConstant(uint32_t index, const uint32_t bits[4]) {
  Pushbuffer::Begin();
  Pushbuffer::Push(NV097_SET_TRANSFORM_CONSTANT_LOAD, index);
  Pushbuffer::PushN(NV097_SET_TRANSFORM_CONSTANT, 4,
                    reinterpret_cast<const DWORD*>(bits));
  Pushbuffer::End();
}

// RDI returns the components of a constant in w, z, y, x order.
static void ReadConstant(uint32_t index, uint32_t out[4]) {
  VIDEOREG(NV_PGRAPH_RDI_INDEX) = kVshConstantsRdiBase + index * 16;
  for (uint32_t component = 0; component < 4; ++component) {
    out[3 - component] = VIDEOREG(NV_PGRAPH_RDI_DATA);
  }
}

static bool RunCompute(ComputeShader shader, const VshConstant* inputs,
                       size_t input_count, uint32_t first_output,
                       size_t output_count, uint32_t (*outputs)[4],
                       const std::function<void(NV2AState&)>& draw) {
  if (!g_state || output_count > kMaxComputeOutputs ||
      first_output + output_count > kVshConstantCount) {
    return false;
  }

  auto& state = *g_state;
  state.SetVertexShaderProgram(g_compute_shaders[static_cast<size_t>(shader)]);
  Pushbuffer::Begin();
  Pushbuffer::Push(NV097_SET_TRANSFORM_PROGRAM_CXT_WRITE_EN, 1);
  Pushbuffer::End();
  state.PrepareDraw(0xFF000000);

  for (size_t i = 0; i < input_count; ++i) {
    LoadConstant(inputs[i].index, inputs[i].bits);
  }
  const uint32_t sentinel[4] = {kVshComputeSentinel, kVshComputeSentinel,
                                kVshComputeSentinel, kVshComputeSentinel};
  for (size_t i = 0; i < output_count; ++i) {
    LoadConstant(first_output + i, sentinel);
  }

  draw(state);

  Pushbuffer::Begin();
  Pushbuffer::Push(NV097_NO_OPERATION, 0);
  Pushbuffer::Push(NV097_WAIT_FOR_IDLE, 0);
  Pushbuffer::End();
  NV2AState::PBKitBusyWait();
  pb_wait_until_gr_not_busy();

  for (size_t i = 0; i < output_count; ++i) {
    ReadConstant(first_output + i, outputs[i]);
  }

  state.SetVertexShaderProgram(nullptr);
  return true;
}

bool VshCompute(ComputeShader shader, const VshConstant* inputs,
                size_t input_count, uint32_t first_output, size_t output_count,
                uint32_t (*outputs)[4]) {
  return RunCompute(shader, inputs, input_count, first_output, output_count,
                    outputs, [](NV2AState& state) {
                      const auto size = static_cast<float>(kProbeSize);
                      state.Begin(NV2AState::PRIMITIVE_QUADS);
                      state.SetVertex(0.0f, 0.0f, 0.1f);
                      state.SetVertex(size, 0.0f, 0.1f);
                      state.SetVertex(size, size, 0.1f);
                      state.SetVertex(0.0f, size, 0.1f);
                      state.End();
                    });
}

bool VshComputeInline(ComputeShader shader, const InlineAttributes& attributes,
                      uint32_t first_output, size_t output_count,
                      uint32_t (*outputs)[4]) {
  return RunCompute(
      shader, nullptr, 0, first_output, output_count, outputs,
      [&attributes](NV2AState& state) {
        const auto& a = attributes;
        state.Begin(NV2AState::PRIMITIVE_QUADS);
        Pushbuffer::Begin();
        for (uint32_t corner = 0; corner < 4; ++corner) {
          Pushbuffer::Push(NV097_SET_NORMAL3F, a.normal[0], a.normal[1],
                           a.normal[2]);
          Pushbuffer::Push(NV097_SET_DIFFUSE_COLOR4F, a.diffuse[0],
                           a.diffuse[1], a.diffuse[2], a.diffuse[3]);
          Pushbuffer::Push(NV097_SET_SPECULAR_COLOR4F, a.specular[0],
                           a.specular[1], a.specular[2], a.specular[3]);
          Pushbuffer::Push(NV097_SET_FOG_COORD, a.fog);
          Pushbuffer::Push(NV097_SET_VERTEX4F, a.position[0], a.position[1],
                           a.position[2], a.position[3]);
        }
        Pushbuffer::End();
        state.End();
      });
}

bool VshComputeArray(ComputeShader shader, const VshConstant* inputs,
                     size_t input_count, const ArrayAttribute& v1,
                     uint32_t first_output, size_t output_count,
                     uint32_t (*outputs)[4]) {
  if (v1.stride > kMaxArrayStride) {
    return false;
  }

  for (uint32_t i = 0; i < kComputeQuadVertices; ++i) {
    memcpy(g_array_v1 + i * v1.stride, v1.data, v1.stride);
  }

  return RunCompute(
      shader, inputs, input_count, first_output, output_count, outputs,
      [&v1](NV2AState&) {
        const uint32_t disabled =
            SET_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE,
                     NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F);
        Pushbuffer::Begin();
        Pushbuffer::Push(NV097_BREAK_VERTEX_BUFFER_CACHE, 0);
        for (uint32_t i = 0; i < kVertexAttributeCount; ++i) {
          Pushbuffer::Push(NV097_SET_VERTEX_DATA_ARRAY_FORMAT + i * 4,
                           disabled);
        }
        Pushbuffer::Push(
            NV097_SET_VERTEX_DATA_ARRAY_FORMAT,
            SET_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE,
                     NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F) |
                SET_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_SIZE, 4) |
                SET_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_STRIDE,
                         4 * sizeof(float)));
        Pushbuffer::Push(NV097_SET_VERTEX_DATA_ARRAY_OFFSET,
                         VRAM_ADDR(g_array_positions));
        Pushbuffer::Push(
            NV097_SET_VERTEX_DATA_ARRAY_FORMAT + 4,
            SET_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE, v1.type) |
                SET_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_SIZE, v1.size) |
                SET_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_STRIDE, v1.stride));
        Pushbuffer::Push(NV097_SET_VERTEX_DATA_ARRAY_OFFSET + 4,
                         VRAM_ADDR(g_array_v1));
        Pushbuffer::Push(NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_QUADS);
        Pushbuffer::Push(
            NV097_DRAW_ARRAYS,
            SET_MASK(NV097_DRAW_ARRAYS_COUNT, kComputeQuadVertices - 1) |
                SET_MASK(NV097_DRAW_ARRAYS_START_INDEX, 0));
        Pushbuffer::Push(NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_END);
        Pushbuffer::Push(NV097_SET_VERTEX_DATA_ARRAY_FORMAT, disabled);
        Pushbuffer::Push(NV097_SET_VERTEX_DATA_ARRAY_FORMAT + 4, disabled);
        Pushbuffer::End();
      });
}

static void DescribeExpected(const ExpectedComponent& e, char* buf,
                             size_t size) {
  if (e.checked) {
    snprintf(buf, size, "0x%08lX", static_cast<unsigned long>(e.bits));
  } else {
    snprintf(buf, size, "*");
  }
}

bool VshCheckVector(const char* label, const uint32_t actual[4],
                    const ExpectedComponent expected[4]) {
  static constexpr const char* kComponents[] = {"x", "y", "z", "w"};
  uint32_t mismatch_mask = 0;
  for (uint32_t i = 0; i < 4; ++i) {
    if (expected[i].checked && actual[i] != expected[i].bits) {
      mismatch_mask |= 1 << i;
    }
  }

  char expected_desc[4][12];
  for (uint32_t i = 0; i < 4; ++i) {
    DescribeExpected(expected[i], expected_desc[i], sizeof(expected_desc[i]));
  }

  LogFields fields = TestFields("label", label);
  for (uint32_t i = 0; i < 4; ++i) {
    fields.Add(kComponents[i], Hex32(actual[i]));
  }
  for (uint32_t i = 0; i < 4; ++i) {
    char key[16];
    snprintf(key, sizeof(key), "expected_%s", kComponents[i]);
    fields.Add(key, expected_desc[i]);
  }
  fields.Add("mismatch_mask", mismatch_mask);

  if (!mismatch_mask) {
    TEST_PASS_EX(fields, "%s: (%g, %g, %g, %g)", label, BitsFloat(actual[0]),
                 BitsFloat(actual[1]), BitsFloat(actual[2]),
                 BitsFloat(actual[3]));
    return true;
  }

  TEST_FAIL_EX(fields,
               "%s: 0x%08lX 0x%08lX 0x%08lX 0x%08lX (%g, %g, %g, %g), expected "
               "%s %s %s %s",
               label, static_cast<unsigned long>(actual[0]),
               static_cast<unsigned long>(actual[1]),
               static_cast<unsigned long>(actual[2]),
               static_cast<unsigned long>(actual[3]), BitsFloat(actual[0]),
               BitsFloat(actual[1]), BitsFloat(actual[2]), BitsFloat(actual[3]),
               expected_desc[0], expected_desc[1], expected_desc[2],
               expected_desc[3]);
  return false;
}
