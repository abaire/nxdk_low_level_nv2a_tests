#ifndef NXDK_LOW_LEVEL_NV2A_TESTS_TEST_UTIL_H
#define NXDK_LOW_LEVEL_NV2A_TESTS_TEST_UTIL_H

#include <hal/fileio.h>
#include <nxdk/format.h>
#include <nxdk/mount.h>
#include <pbkit/pbkit.h>
#include <windows.h>

#include <cassert>
#include <cstdint>
#include <string>

#define MAX_FILE_PATH_SIZE 248

constexpr int kFramebufferWidth = 640;
constexpr int kFramebufferHeight = 480;
constexpr int kBitsPerPixel = 32;

static inline void ClearScreen(uint32_t color = 0) {
  pb_erase_depth_stencil_buffer(0, 0, kFramebufferWidth, kFramebufferHeight);
  pb_fill(0, 0, kFramebufferWidth, kFramebufferHeight, color);
  pb_erase_text_screen();
}

#if defined(__cplusplus)
extern "C" {
#endif

// Version of pb_print that uses a full-featured printf implementation instead
// of the PDCLIB one that does not support floats.
void pb_print_with_floats(const char* format, ...);
#define pb_print pb_print_with_floats

#if defined(__cplusplus)
}  // extern "C"
#endif

static void EnsureFolderExists(const std::string& folder_path) {
  if (folder_path.length() > MAX_FILE_PATH_SIZE) {
    assert(!"Folder Path is too long.");
  }

  char buffer[MAX_FILE_PATH_SIZE + 1] = {0};
  const char* path_start = folder_path.c_str();
  const char* slash = strchr(path_start, '\\');
  if (slash) {
    slash = strchr(slash + 1, '\\');
  }

  while (slash) {
    strncpy(buffer, path_start, slash - path_start);
    buffer[slash - path_start] = 0;
    if (!CreateDirectory(buffer, nullptr) &&
        GetLastError() != ERROR_ALREADY_EXISTS) {
      assert(!"Failed to create output directory.");
    }

    slash = strchr(slash + 1, '\\');
  }

  // Handle case where there was no trailing slash.
  if (!CreateDirectory(path_start, nullptr) &&
      GetLastError() != ERROR_ALREADY_EXISTS) {
    assert(!"Failed to create output directory.");
  }
}

static bool EnsureDriveMounted(char drive_letter, bool format = false) {
  if (nxIsDriveMounted(drive_letter)) {
    return true;
  }

  char dos_path[4] = "x:\\";
  dos_path[0] = drive_letter;
  char device_path[256] = {0};
  if (XConvertDOSFilenameToXBOX(dos_path, device_path) != STATUS_SUCCESS) {
    return false;
  }

  if (!strstr(device_path, R"(\Device\Harddisk0\Partition)")) {
    return false;
  }
  device_path[28] = 0;

  if (format) {
    char last_char = device_path[27];
    if (last_char != '3' && last_char != '4' && last_char != '5') {
      return false;
    }
    if (!nxFormatVolume(device_path, 0)) {
      return false;
    }
  }

  return nxMountDrive(drive_letter, device_path);
}

#endif  // NXDK_LOW_LEVEL_NV2A_TESTS_TEST_UTIL_H
