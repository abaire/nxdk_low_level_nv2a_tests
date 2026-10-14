#include "m2mf_test_common.h"

#include <hal/debug.h>
#include <pbkit/nv_objects.h>
#include <pbkit/pbkit.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <cstring>

#include "test_log.h"

bool M2MFInit() {
  PBKitPlusPlus::Pushbuffer::Initialize();
  if (PBKitPlusPlus::NV2AState::InitializeM2M() < 0) {
    TEST_ERROR("gpum_init failed!");
    return false;
  }
  return true;
}

void M2MFTeardown() { PBKitPlusPlus::NV2AState::PBKitBusyWait(); }

uint32_t M2MFGetPhysicalAddress(const void* p) {
  uintptr_t va = reinterpret_cast<uintptr_t>(p);
  if (va >= 0x80000000 && va < 0xC0000000) {
    return static_cast<uint32_t>(va & 0x1FFFFFFF);
  }
  return static_cast<uint32_t>(
      reinterpret_cast<uintptr_t>(MmGetPhysicalAddress(const_cast<PVOID>(p))));
}

void M2MFFillPattern(void* buf, size_t size, uint8_t seed) {
  uint8_t* b = static_cast<uint8_t*>(buf);
  uint8_t val = seed;
  for (size_t i = 0; i < size; ++i) {
    b[i] = val;
    val = static_cast<uint8_t>((val * 109 + 89) ^ (i & 0xFF));
  }
}

bool M2MFVerifyPattern(const void* buf, size_t size, uint8_t seed) {
  const uint8_t* b = static_cast<const uint8_t*>(buf);
  uint8_t val = seed;
  for (size_t i = 0; i < size; ++i) {
    if (b[i] != val) {
      return false;
    }
    val = static_cast<uint8_t>((val * 109 + 89) ^ (i & 0xFF));
  }
  return true;
}

bool M2MFVerifyCanaries(const void* buf, size_t size, uint8_t canary) {
  const uint8_t* b = static_cast<const uint8_t*>(buf);
  for (size_t i = 0; i < size; ++i) {
    if (b[i] != canary) {
      return false;
    }
  }
  return true;
}
