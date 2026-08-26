#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_PBKIT_UTIL_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_PBKIT_UTIL_H

#include <pbkit/pbkit.h>

#include "printf/printf.h"

constexpr int kFramebufferWidth = 640;
constexpr int kFramebufferHeight = 480;
constexpr int kBitsPerPixel = 32;

static inline void PBKitBusyWait() {
  while (pb_busy()) {
    /* Wait for completion... */
  }
}

static inline void PBKitFlip() {
  while (pb_finished()) {
    /* Wait for completion... */
  }
}

static inline void PBKitClearScreen(uint32_t color) {
  pb_erase_depth_stencil_buffer(0, 0, kFramebufferWidth, kFramebufferHeight);
  pb_fill(0, 0, kFramebufferWidth, kFramebufferHeight, color);
  pb_erase_text_screen();
}

#if defined(__cplusplus)
extern "C" {
#endif

// Versions of pb_print that use a full-featured printf implementation instead
// of the PCDLIB one that does not yet support floats.
void pb_print_with_floats(const char* format, ...);
#define pb_print pb_print_with_floats

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_PBKIT_UTIL_H
