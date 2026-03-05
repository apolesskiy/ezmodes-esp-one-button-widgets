#pragma once

/**
 * @file menu_config.hpp
 * @brief Configuration struct for the menu system.
 *
 * Holds font pointers and derives layout metrics (item height)
 * from the font, keeping the menu system independent of any
 * particular font choice.
 */

#include "lvgl.h"

#include <cstdint>

namespace ezmodes {
namespace ui {

/// Default item height when no font is configured (pixels).
inline constexpr int32_t kDefaultItemHeight = 10;

/**
 * @brief Runtime configuration for a Menu and its child widgets.
 *
 * Pass an instance to the Menu constructor. The Menu propagates
 * the primary font to every child widget before rendering.
 */
struct MenuConfig {
  /// Primary (large) font used for menu item labels.
  const lv_font_t* font_large = nullptr;

  /// Secondary (small) font available for value hints or status text.
  const lv_font_t* font_small = nullptr;

  /// Derive item row height from the primary font.
  int32_t item_height() const {
    return font_large ? font_large->line_height : kDefaultItemHeight;
  }
};

}  // namespace ui
}  // namespace ezmodes
