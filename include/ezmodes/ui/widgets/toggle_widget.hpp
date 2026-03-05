#pragma once

/**
 * @file toggle_widget.hpp
 * @brief Boolean toggle widget.
 *
 * Displays label with on/off indicator. Long press toggles state.
 */

#include "ezmodes/ui/widget.hpp"

#include <functional>

namespace ezmodes {
namespace ui {

// Forward declaration for callback
class ToggleWidget;

/**
 * @brief Callback when toggle state changes.
 * @param widget Reference to the toggle
 * @param new_state The new boolean state
 */
using ToggleCallback = std::function<void(ToggleWidget& widget, bool new_state)>;

/**
 * @brief Boolean toggle widget.
 *
 * Displays label with on/off indicator. Long press toggles state.
 *
 * Input behavior:
 * - Short press: Returns kFocusNext (move to next item)
 * - Long press: Toggles state, invokes on_changed callback, returns kConsumed
 */
class ToggleWidget : public Widget {
 public:
  /**
   * @brief Construct a toggle widget with external value storage.
   * @param label Display text (must remain valid for widget lifetime)
   * @param value_ptr Pointer to boolean storage (modified directly)
   */
  ToggleWidget(const char* label, bool* value_ptr);

  /**
   * @brief Construct a toggle widget with internal value storage.
   * @param label Display text (must remain valid for widget lifetime)
   * @param initial_value Starting value
   */
  ToggleWidget(const char* label, bool initial_value = false);

  ~ToggleWidget() override;

  // Prevent copying
  ToggleWidget(const ToggleWidget&) = delete;
  ToggleWidget& operator=(const ToggleWidget&) = delete;

  // --- Event Subscription ---

  /**
   * @brief Set callback for when toggle state changes.
   */
  void set_on_changed(ToggleCallback callback);

  // --- Accessors ---

  /**
   * @brief Get the label text.
   */
  const char* get_label() const { return label_; }

  /**
   * @brief Get current value.
   */
  bool get_value() const;

  /**
   * @brief Set value directly.
   */
  void set_value(bool value);

  // --- Display Customization ---

  /**
   * @brief Set custom text for on/off states.
   * @param on_text Text to display when true (default: "On")
   * @param off_text Text to display when false (default: "Off")
   */
  void set_state_text(const char* on_text, const char* off_text);

  // --- Widget Interface ---

  void render(lv_obj_t* parent, int32_t y_offset) override;
  void destroy() override;
  void set_focused(bool focused) override;
  bool is_focused() const override { return focused_; }
  InputResult handle_input(bool short_press) override;

 private:
  const char* label_;

  // Value storage - either external pointer or internal
  bool* value_ptr_;
  bool internal_value_;
  bool owns_value_;  // true if using internal_value_

  const char* on_text_ = "On";
  const char* off_text_ = "Off";

  lv_obj_t* lv_label_ = nullptr;
  bool focused_ = false;
  ToggleCallback on_changed_;

  /**
   * @brief Update display text.
   */
  void update_display();

  /**
   * @brief Update visual appearance based on focus state.
   */
  void update_appearance();

  /**
   * @brief Get pointer to current value (external or internal).
   */
  bool* value_storage();
};

}  // namespace ui
}  // namespace ezmodes
