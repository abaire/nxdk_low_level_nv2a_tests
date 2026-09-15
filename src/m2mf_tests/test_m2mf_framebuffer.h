#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_FRAMEBUFFER_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_FRAMEBUFFER_H

//! Validates direct DMA transfers from system RAM to the active display
//! framebuffer surface.
//!
//! Validates that:
//! 1. Memory transfers directly to the active hardware display backbuffer
//! aperture succeed.
//! 2. Display buffer contents accurately reflect the transferred source
//! pattern.
struct TestM2MFFramebuffer {
  static constexpr const char* Name() {
    return "M2MF Framebuffer Aperture Write";
  }
  static bool Test();
};

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_M2MF_FRAMEBUFFER_H
