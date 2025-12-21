#ifndef XBOX
#error Must be built with nxdk
#endif

#include <SDL.h>
#include <hal/debug.h>
#include <hal/video.h>
#include <pbkit/pbkit.h>
#include <windows.h>

extern "C" DWORD ptimer_alarm_count;

static const int kFramebufferWidth = 640;
static const int kFramebufferHeight = 480;
static const int kBitsPerPixel = 32;

/* Main program function */
int main() {
  debugPrint("Set video mode");
  if (!XVideoSetMode(kFramebufferWidth, kFramebufferHeight, kBitsPerPixel, REFRESH_DEFAULT)) {
    debugPrint("Failed to set video mode\n");
    Sleep(2000);
    return 1;
  }

  int status = pb_init();
  if (status) {
    debugPrint("pb_init Error %d\n", status);
    Sleep(2000);
    return 1;
  }

  debugPrint("Initializing...");
  pb_show_debug_screen();

  if (SDL_Init(SDL_INIT_GAMECONTROLLER)) {
    debugPrint("Failed to initialize SDL_GAMECONTROLLER.");
    debugPrint("%s", SDL_GetError());
    pb_show_debug_screen();
    Sleep(2000);
    return 1;
  }

  pb_show_front_screen();
  debugClearScreen();

  static constexpr float kQuadSize = 250.f;
  static constexpr float kQuadLeft = (kFramebufferWidth - kQuadSize) * 0.5f;
  static constexpr float kQuadRight = kQuadLeft + kQuadSize;
  static constexpr float kQuadTop = (kFramebufferHeight - kQuadSize) * 0.5f;
  static constexpr float kQuadBottom = kQuadTop + kQuadSize;
  static constexpr float kQuadZ = 1.f;
  static constexpr float kQuadW = 1.f;

  DWORD gpu_tick_low = VIDEOREG(NV_PTIMER_TIME_0);
  VIDEOREG(NV_PTIMER_TIME_1) = 1;
  VIDEOREG(NV_PTIMER_ALARM_0) = 0xFFFFFFFF;
  VIDEOREG(NV_PTIMER_INTR_EN_0) = 1;

  bool running = true;
  while (running) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED: {
          SDL_GameController *controller = SDL_GameControllerOpen(event.cdevice.which);
          if (!controller) {
            debugPrint("Failed to handle controller add event.");
            debugPrint("%s", SDL_GetError());
            running = false;
          }
        } break;

        case SDL_CONTROLLERDEVICEREMOVED: {
          SDL_GameController *controller = SDL_GameControllerFromInstanceID(event.cdevice.which);
          SDL_GameControllerClose(controller);
        } break;

        case SDL_CONTROLLERBUTTONUP:
          running = false;
          break;

        default:
          break;
      }
    }

    pb_wait_for_vbl();
    pb_reset();
    pb_target_back_buffer();

    /* Clear depth & stencil buffers */
    pb_erase_depth_stencil_buffer(0, 0, kFramebufferWidth, kFramebufferHeight);
    pb_fill(0, 0, kFramebufferWidth, kFramebufferHeight, 0x00000000);
    pb_erase_text_screen();

    while (pb_busy()) {
      /* Wait for completion... */
    }

    {
      auto p = pb_begin();
      p = pb_push1(p, NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_QUADS);
      p = pb_push1(p, NV097_SET_DIFFUSE_COLOR4I, 0xFFFF0000);
      p = pb_push4f(p, NV097_SET_VERTEX4F, kQuadLeft, kQuadTop, kQuadZ, kQuadW);

      p = pb_push1(p, NV097_SET_DIFFUSE_COLOR4I, 0xFF00FF00);
      p = pb_push4f(p, NV097_SET_VERTEX4F, kQuadRight, kQuadTop, kQuadZ, kQuadW);

      p = pb_push1(p, NV097_SET_DIFFUSE_COLOR4I, 0xFF0000FF);
      p = pb_push4f(p, NV097_SET_VERTEX4F, kQuadRight, kQuadBottom, kQuadZ, kQuadW);

      p = pb_push1(p, NV097_SET_DIFFUSE_COLOR4I, 0xFF7F7F7F);
      p = pb_push4f(p, NV097_SET_VERTEX4F, kQuadLeft, kQuadBottom, kQuadZ, kQuadW);

      p = pb_push1(p, NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_END);
      pb_end(p);
    }

    pb_print("Press any button to exit\n");
    pb_print("alarm reg = 0x%X\n", VIDEOREG(NV_PTIMER_ALARM_0));
    pb_print("ptimer_alarm_count = %d\n", ptimer_alarm_count);

    // VIDEOREG(NV_PTIMER_TIME_0) = 0x45;
    DWORD foo = VIDEOREG(NV_PTIMER_TIME_0);
    DWORD bar = VIDEOREG(NV_PTIMER_TIME_1);
    pb_print("time_0 reg = 0x%X\n", foo);
    pb_print("time_1 reg = 0x%X\n", bar);

    pb_draw_text_screen();

    while (pb_busy()) {
    }
    while (pb_finished()) {
    }
  }

  pb_kill();
  return 0;
}
