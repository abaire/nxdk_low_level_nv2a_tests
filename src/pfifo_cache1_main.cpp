#ifndef XBOX
#error Must be built with nxdk
#endif

#include <SDL.h>
#include <hal/debug.h>
#include <hal/video.h>
#include <pbkit/pbkit.h>
#include <windows.h>

// mmio blocks
#define NV2A_MMIO_BASE 0xFD000000
#define BLOCK_PMC 0x000000
#define BLOCK_PBUS 0x001000
#define BLOCK_PFIFO 0x002000
#define BLOCK_PRMA 0x007000
#define BLOCK_PVIDEO 0x008000
#define BLOCK_PTIMER 0x009000
#define BLOCK_PCOUNTER 0x00A000
#define BLOCK_PVPE 0x00B000
#define BLOCK_PTV 0x00D000
#define BLOCK_PRMFB 0x0A0000
#define BLOCK_PRMVIO 0x0C0000
#define BLOCK_PFB 0x100000
#define BLOCK_PSTRAPS 0x101000
#define BLOCK_PGRAPH 0x400000
#define BLOCK_PCRTC 0x600000
#define BLOCK_PRMCIO 0x601000
#define BLOCK_PRAMDAC 0x680000
#define BLOCK_PRMDIO 0x681000
#define BLOCK_PRAMIN 0x700000
#define BLOCK_USER 0x800000

#define _PFIFO_ADDR(addr) (NV2A_MMIO_BASE + (addr))
// #define _PGRAPH_ADDR(addr) (NV2A_MMIO_BASE + BLOCK_PGRAPH + (addr))

extern "C" {
extern DWORD pb_Size;
extern uint32_t* pb_Head;
extern uint32_t* pb_Tail;
extern uint32_t* pb_Put;
extern DWORD pb_PushBase;
extern DWORD pb_PushLimit;
extern volatile DWORD* pb_DmaUserAddr;
extern DWORD pb_PushIndex;
extern DWORD* pb_PushStart;
extern DWORD* pb_PushNext;
extern DWORD pb_FBAddr[3];
extern int pb_front_index;
extern int pb_back_index;
void set_draw_buffer(DWORD buffer_addr);
}

static const auto kMillisecondsBetweenTests = 1000;
static const int kFramebufferWidth = 640;
static const int kFramebufferHeight = 480;
static const int kBitsPerPixel = 32;

volatile DWORD* USER_DMA_PUT =
    reinterpret_cast<DWORD*>(VIDEO_BASE + NV_USER + 0x40);
volatile DWORD* USER_DMA_GET =
    reinterpret_cast<DWORD*>(VIDEO_BASE + NV_USER + 0x44);

#define DMA_STATE _PFIFO_ADDR(NV_PFIFO_CACHE1_DMA_STATE)
#define DMA_PUT_ADDR _PFIFO_ADDR(NV_PFIFO_CACHE1_DMA_PUT)
#define DMA_GET_ADDR _PFIFO_ADDR(NV_PFIFO_CACHE1_DMA_GET)
#define DMA_SUBROUTINE _PFIFO_ADDR(NV_PFIFO_CACHE1_DMA_SUBROUTINE)

#define CACHE_PUSH_MASTER_STATE _PFIFO_ADDR(NV_PFIFO_CACHE1_PUSH0)
#define CACHE_PUSH_STATE _PFIFO_ADDR(NV_PFIFO_CACHE1_DMA_PUSH)
#define CACHE_PULL_STATE _PFIFO_ADDR(NV_PFIFO_CACHE1_PULL0)
#define CACHE_PUT_ADDR _PFIFO_ADDR(NV_PFIFO_CACHE1_PUT)
#define CACHE_GET_ADDR _PFIFO_ADDR(NV_PFIFO_CACHE1_GET)
#define CACHE1_STATUS _PFIFO_ADDR(NV_PFIFO_CACHE1_STATUS)

#define CACHE1_METHOD _PFIFO_ADDR(NV_PFIFO_CACHE1_METHOD)
#define CACHE1_DATA _PFIFO_ADDR(NV_PFIFO_CACHE1_DATA)
#define RAM_HASHTABLE _PFIFO_ADDR(NV_PFIFO_RAMHT)

#define CTX_SWITCH1 _PGRAPH_ADDR(NV_PGRAPH_CTX_SWITCH1)
#define PGRAPH_STATE _PGRAPH_ADDR(NV_PGRAPH_FIFO)

inline uint32_t ReadDWORD(intptr_t address) {
  return *(volatile uint32_t*)(address);
}

inline void WriteDWORD(intptr_t address, uint32_t value) {
  *(volatile uint32_t*)(address) = value;
}

static void pb_cache_flush() {
  __asm__ __volatile__("sfence");
  // assembler instruction "sfence" : waits end of previous instructions

  VIDEOREG(NV_PFB_WC_CACHE) |= NV_PFB_WC_CACHE_FLUSH_TRIGGER;
  while (VIDEOREG(NV_PFB_WC_CACHE) & NV_PFB_WC_CACHE_FLUSH_IN_PROGRESS) {
  };
}

struct StateEntry {
  DWORD dma_get;
  DWORD dma_put;
  DWORD cache_get;
  DWORD cache_put;

  DWORD cache_push_state;
  DWORD cache_pull_state;
  DWORD cache1_status;
};

static constexpr auto kStateBufferEntries = 4096;
static StateEntry* state_buffer = nullptr;

inline void FillStateBuffer(StateEntry* state_entry) {
  for (auto i = 0; i < kStateBufferEntries; ++i, ++state_entry) {
    state_entry->dma_get = ReadDWORD(DMA_GET_ADDR);
    state_entry->dma_put = ReadDWORD(DMA_PUT_ADDR);
    state_entry->cache_get = ReadDWORD(CACHE_GET_ADDR);
    state_entry->cache_put = ReadDWORD(CACHE_PUT_ADDR);

    state_entry->cache_push_state = ReadDWORD(CACHE_PUSH_STATE);
    state_entry->cache_pull_state = ReadDWORD(CACHE_PULL_STATE);
    state_entry->cache1_status = ReadDWORD(CACHE1_STATUS);
  }
}

void PrintStateBuffer(const StateEntry* state_entry) {
  StateEntry last_entry_set = {0};
  DWORD num_repeats = 0;

  for (auto i = 0; i < kStateBufferEntries; ++i, ++state_entry) {
    if (i && !memcmp(&last_entry_set, state_entry, sizeof(last_entry_set))) {
      ++num_repeats;
      continue;
    }

    memcpy(&last_entry_set, state_entry, sizeof(last_entry_set));
    if (num_repeats) {
      DbgPrint("\t    ... repeated %d times ...\n", num_repeats);
      num_repeats = 0;
    }

    DbgPrint(
        "\t%d DMA: GET 0x%08X PUT 0x%08X  CACHE1: GET 0x%08X PUT 0x%08X "
        "CachePushState: 0x%08X PullState: 0x%08X cache1status: 0x%08X\n",
        i, state_entry->dma_get, state_entry->dma_put, state_entry->cache_get,
        state_entry->cache_put, state_entry->cache_push_state,
        state_entry->cache_pull_state, state_entry->cache1_status);
  }

  if (num_repeats) {
    DbgPrint("\t    ... repeated %d times ...\n", num_repeats);
  }
}

void PrintCurrentState() {
  StateEntry state_entry = {.dma_get = ReadDWORD(DMA_GET_ADDR),
                            .dma_put = ReadDWORD(DMA_PUT_ADDR),
                            .cache_get = ReadDWORD(CACHE_GET_ADDR),
                            .cache_put = ReadDWORD(CACHE_PUT_ADDR),
                            .cache_push_state = ReadDWORD(CACHE_PUSH_STATE),
                            .cache_pull_state = ReadDWORD(CACHE_PULL_STATE),
                            .cache1_status = ReadDWORD(CACHE1_STATUS)};

  DbgPrint(
      "Current state: DMA: GET 0x%08X PUT 0x%08X  CACHE1: GET 0x%08X PUT "
      "0x%08X "
      "CachePushState: 0x%08X PullState: 0x%08X cache1status: 0x%08X\n",
      state_entry.dma_get, state_entry.dma_put, state_entry.cache_get,
      state_entry.cache_put, state_entry.cache_push_state,
      state_entry.cache_pull_state, state_entry.cache1_status);
}

void CommitPushbuffer(uint32_t* p) {
  pb_Put = p;
  pb_cache_flush();
  *USER_DMA_PUT = reinterpret_cast<DWORD>(pb_Put) & 0x03FFFFFF;
}

/* Main program function */
int main() {
  state_buffer = new StateEntry[kStateBufferEntries];

  debugPrint("Set video mode");
  if (!XVideoSetMode(kFramebufferWidth, kFramebufferHeight, kBitsPerPixel,
                     REFRESH_DEFAULT)) {
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

  // Target the front buffer so all rendering is immediately visible without
  // dealing with flip/stall/etc...
  set_draw_buffer(pb_FBAddr[pb_front_index] & 0x03FFFFFF);

  // Prove that neither the DMA pull nor the CACHE1 pointers move until the
  // PIO get/put is updated.
  {
    DbgPrint(
        "This test submits a tiny pushbuffer that sets the clear color value "
        "and clears the active surface\n");

    PrintCurrentState();

    auto p = pb_begin();
    p = pb_push1(p, NV097_SET_COLOR_CLEAR_VALUE, 0x7F7F7F7F);
    p = pb_push1(p, NV097_CLEAR_SURFACE,
                 NV097_CLEAR_SURFACE_COLOR | NV097_CLEAR_SURFACE_STENCIL |
                     NV097_CLEAR_SURFACE_Z);

    DbgPrint(
        "At this point the pushbuffer has been created in system memory but "
        "has not been submitted yet. The current DMA and CACHE1 buffer "
        "pointers will now be captured repeatedly and printed.\n");
    FillStateBuffer(state_buffer);
    PrintStateBuffer(state_buffer);
    Sleep(500);

    DbgPrint(
        "Now the pushbuffer will be submitted by modifying the DMA PUT via "
        "NV_USER.\n");
    Sleep(500);

    // This will commit the buffer and cause it to be read.
    // This also enables the CACHE1 to be consumed such that the
    // commands are executed.
    CommitPushbuffer(p);
    FillStateBuffer(state_buffer);

    DbgPrint("DMA/CACHE1 state immediately following the commit:\n");
    PrintStateBuffer(state_buffer);

    DbgPrint(
        "Test completed, sleeping and resetting the pushbuffer pointers\n");
    Sleep(kMillisecondsBetweenTests);
    pb_reset();
  }

  {
    DbgPrint(
        "This test submits a much larger buffer in several batches. The buffer "
        "is primarily made up of NOP or WAIT_FOR_IDLE commands with a few "
        "clear surface calls interspersed.\n");
    PrintCurrentState();

    auto p = pb_begin();

    for (auto i = 0; i < 16; ++i) {
      p = pb_push1(p, NV097_NO_OPERATION, 1);
    }
    p = pb_push1(p, NV097_SET_COLOR_CLEAR_VALUE, 0xFFFF0000);
    p = pb_push1(p, NV097_CLEAR_SURFACE,
                 NV097_CLEAR_SURFACE_COLOR | NV097_CLEAR_SURFACE_STENCIL |
                     NV097_CLEAR_SURFACE_Z);

    DbgPrint("About to commit NOPs followed by a CLEAR_SURFACE...\n");
    Sleep(500);
    CommitPushbuffer(p);
    FillStateBuffer(state_buffer);

    DbgPrint("DMA/CACHE1 state after the first submission:\n");
    PrintStateBuffer(state_buffer);

    DbgPrint(
        "About to push NOPs, another CLEAR_SURFACE, and WAIT_FOR_IDLE "
        "commands\n");
    for (auto i = 0; i < 32; ++i) {
      p = pb_push1(p, NV097_NO_OPERATION, 1);
    }
    p = pb_push1(p, NV097_SET_COLOR_CLEAR_VALUE, 0xFF0000FF);
    p = pb_push1(p, NV097_CLEAR_SURFACE,
                 NV097_CLEAR_SURFACE_COLOR | NV097_CLEAR_SURFACE_STENCIL |
                     NV097_CLEAR_SURFACE_Z);
    for (auto i = 0; i < 16; ++i) {
      p = pb_push1(p, NV097_WAIT_FOR_IDLE, 0);
    }

    DbgPrint("State prior to committing the next batch:\n");
    FillStateBuffer(state_buffer);
    PrintStateBuffer(state_buffer);
    Sleep(1000);

    CommitPushbuffer(p);
    FillStateBuffer(state_buffer);

    DbgPrint("State after committing the batch:\n");
    PrintStateBuffer(state_buffer);

    Sleep(kMillisecondsBetweenTests);
    pb_reset();
  }

  {
    DbgPrint(
        "This test submits batches in a loop, resetting the DMA pointers in "
        "between submissions.\n");

    DbgPrint("DMA/CACHE1 state prior to the first submission:\n");
    PrintCurrentState();

    constexpr auto kNumLoops = 32;
    StateEntry* state_buffers[kNumLoops];
    for (auto& buffer : state_buffers) {
      buffer = new StateEntry[kStateBufferEntries];
    }

    constexpr auto kPushSetsPerLoop = 52;
    constexpr auto kWordsPerSet = 2;

    for (auto loop = 0; loop < kNumLoops; ++loop) {
      auto p = pb_begin();

      for (auto i = 0; i < kPushSetsPerLoop; ++i) {
        p = pb_push1(p, NV097_SET_COLOR_CLEAR_VALUE, 0xFFFF0000 + loop * 64);
        p = pb_push1(p, NV097_CLEAR_SURFACE,
                     NV097_CLEAR_SURFACE_COLOR | NV097_CLEAR_SURFACE_STENCIL |
                         NV097_CLEAR_SURFACE_Z);
      }

      pb_end(p);
      FillStateBuffer(state_buffers[loop]);

      pb_reset();
    }

    for (auto loop = 0; loop < kNumLoops; ++loop) {
      DbgPrint(
          "DMA/CACHE1 state after submission %d [%d elements = %d bytes]\n",
          loop, kPushSetsPerLoop * kWordsPerSet,
          kPushSetsPerLoop * kWordsPerSet * 4);
      PrintStateBuffer(state_buffers[loop]);
    }

    for (auto& buffer : state_buffers) {
      delete[] buffer;
      buffer = nullptr;
    }

    Sleep(kMillisecondsBetweenTests);

    DbgPrint("State after final sleep\n");
    FillStateBuffer(state_buffer);
    PrintStateBuffer(state_buffer);

    pb_reset();
  }

  pb_kill();
  return 0;
}
