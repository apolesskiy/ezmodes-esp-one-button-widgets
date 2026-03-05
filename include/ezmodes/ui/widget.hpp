#pragma once

/**
 * @file widget.hpp
 * @brief Base interface for all UI widgets in the menu system.
 *
 * Widgets are self-contained UI elements that render themselves,
 * handle input when focused, and emit events via callbacks.
 */

#include "lvgl.h"

#include <cstdint>

#include "ezmodes/ui/menu_config.hpp"

namespace ezmodes {
namespace ui {

/**
 * @brief Result of handling input, tells controller what to do next.
 */
enum class InputResult {
  kConsumed,    ///< Input handled, no further action needed
  kFocusNext,   ///< Move focus to next widget (short press default)
  kFocusPrev,   ///< Move focus to previous widget
  kSelected,    ///< Widget was selected (long press), may trigger navigation
  kDeselected,  ///< Widget exited active/edit state
  kNotHandled,  ///< Input not consumed, propagate to parent
};

/**
 * @brief Base interface for all UI widgets.
 *
 * Widgets are self-contained UI elements that:
 * - Render themselves to an LVGL parent
 * - Handle input when focused
 * - Emit events via callbacks set by the consumer
 *
 * The MenuController uses this interface to manage widgets without
 * knowing their concrete types.
 */
class Widget {
 public:
  virtual ~Widget() = default;

  // --- Lifecycle ---

  /**
   * @brief Render the widget to the given parent container.
   * @param parent LVGL parent object
   * @param y_offset Vertical position in pixels
   */
  virtual void render(lv_obj_t* parent, int32_t y_offset) = 0;

  /**
   * @brief Remove widget from display and clean up LVGL objects.
   */
  virtual void destroy() = 0;

  // --- Focus Management (used by MenuController) ---

  /**
   * @brief Update visual state when focus changes.
   * @param focused Whether this widget currently has focus
   */
  virtual void set_focused(bool focused) = 0;

  /**
   * @brief Check if this widget has focus.
   * @return true if focused
   */
  virtual bool is_focused() const = 0;

  /**
   * @brief Check if this widget can receive focus.
   * @return true if focusable (most widgets), false for decorative elements
   */
  virtual bool is_focusable() const { return true; }

  // --- Input Handling ---

  /**
   * @brief Handle button input when this widget has focus.
   * @param short_press true for short press, false for long press
   * @return InputResult indicating how the input was handled
   */
  virtual InputResult handle_input(bool short_press) = 0;

  // --- Font ---

  /**
   * @brief Set the font for this widget (called by Menu before render).
   * @param font Primary font pointer
   */
  void set_font(const lv_font_t* font) { font_ = font; }

  /**
   * @brief Get the font assigned to this widget.
   */
  const lv_font_t* get_font() const { return font_; }

  // --- Layout ---

  /**
   * @brief Get the height of this widget in pixels.
   *
   * Returns the font line height if a font has been set,
   * otherwise falls back to kDefaultItemHeight.
   */
  virtual int32_t get_height() const {
    return font_ ? font_->line_height : kDefaultItemHeight;
  }

 protected:
  /// Font set by the owning Menu; nullptr until set_font() is called.
  const lv_font_t* font_ = nullptr;
};

}  // namespace ui
}  // namespace ezmodes
