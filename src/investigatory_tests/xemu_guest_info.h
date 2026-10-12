#ifndef FETCH_XEMU_VERSION_H
#define FETCH_XEMU_VERSION_H

#ifdef __cplusplus
#include <cstdbool>
#include <cstdint>
#else
#include <stdbool.h>
#include <stdint.h>
#endif

#define XEMU_GUEST_INFO_IO_INDEX 0x04F4
#define XEMU_GUEST_INFO_IO_DATA (XEMU_GUEST_INFO_IO_INDEX + 1)

#define XEMU_INFO_KEY_MAGIC 0x00
#define XEMU_INFO_KEY_VERSION_MAJOR 0x01
#define XEMU_INFO_KEY_VERSION_MINOR 0x02
#define XEMU_INFO_KEY_VERSION_PATCH 0x03

/* "XEMU" */
#define XEMU_MAGIC_VAL 0x554D4558

typedef struct {
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
} XemuVersion;

static inline void __xemu_out8(uint16_t port, uint8_t val) {
  __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t __xemu_in32(uint16_t port) {
  uint32_t ret;
  __asm__ volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static inline bool xemuinfo_host_is_xemu(void) {
  __xemu_out8(XEMU_GUEST_INFO_IO_INDEX, XEMU_INFO_KEY_MAGIC);
  return __xemu_in32(XEMU_GUEST_INFO_IO_DATA) == XEMU_MAGIC_VAL;
}

static inline bool xemuinfo_get_version(XemuVersion* out_ver) {
  if (!xemuinfo_host_is_xemu() || !out_ver) {
    return false;
  }
  __xemu_out8(XEMU_GUEST_INFO_IO_INDEX, XEMU_INFO_KEY_VERSION_MAJOR);
  out_ver->major = __xemu_in32(XEMU_GUEST_INFO_IO_DATA);

  __xemu_out8(XEMU_GUEST_INFO_IO_INDEX, XEMU_INFO_KEY_VERSION_MINOR);
  out_ver->minor = __xemu_in32(XEMU_GUEST_INFO_IO_DATA);

  __xemu_out8(XEMU_GUEST_INFO_IO_INDEX, XEMU_INFO_KEY_VERSION_PATCH);
  out_ver->patch = __xemu_in32(XEMU_GUEST_INFO_IO_DATA);

  return true;
}

#endif /* XEMU_GUEST_VERSION_H */
