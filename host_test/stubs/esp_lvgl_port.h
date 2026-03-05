/**
 * @file esp_lvgl_port.h
 * @brief Stub for esp_lvgl_port for host testing.
 */

#pragma once

#include <cstdint>

inline bool lvgl_port_lock(uint32_t timeout_ms) {
  (void)timeout_ms;
  return true;
}

inline void lvgl_port_unlock() {}
