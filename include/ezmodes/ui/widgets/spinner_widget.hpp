#pragma once

/**
 * @file spinner_widget.hpp
 * @brief Editable spinner widget for cyclic value selection.
 *
 * Two modes:
 * - Focused (not editing): Highlighted, long press enters edit mode
 * - Editing: Short press cycles value, long press commits and exits
 */

#include "ezmodes/ui/widget.hpp"

#include <functional>

namespace ezmodes {
namespace ui {

// Forward declaration for callbacks
class SpinnerWidget;

/**
 * @brief Callback when spinner value changes (during editing).
 * @param widget Reference to the spinner
 * @param new_value The new value after cycling
 */
using SpinnerChangeCallback = std::function<void(SpinnerWidget& widget, int32_t new_value)>;

/**
 * @brief Callback when spinner editing is complete (deselected).
 * @param widget Reference to the spinner
 * @param final_value The committed value
 */
using SpinnerCommitCallback = std::function<void(SpinnerWidget& widget, int32_t final_value)>;

/**
 * @brief Callback to format the spinner display text.
 * @param value Current value
 * @param buffer Output buffer for formatted text
 * @param buffer_size Size of output buffer
 */
using SpinnerFormatCallback = std::function<void(int32_t value, char* buffer, size_t buffer_size)>;

/**
 * @brief Editable spinner widget for cyclic value selection.
 *
 * Consumer provides callbacks for:
 * - on_changed: Called each time value cycles (for live preview)
 * - on_commit: Called when editing is finished
 * - format: Called to generate display text
 *
 * Value can be stored either:
 * - Externally via pointer (value_ptr constructor)
 * - Internally (initial_value constructor)
 */
class SpinnerWidget : public Widget {
 public:
  /**
   * @brief Construct a spinner widget with external value storage.
   * @param value_ptr Pointer to the value storage (modified directly)
   * @param min_value Minimum value (inclusive)
   * @param max_value Maximum value (inclusive)
   * @param step Increment/decrement step
   */
  SpinnerWidget(int32_t* value_ptr, int32_t min_value, int32_t max_value, int32_t step);

  /**
   * @brief Construct a spinner widget with internal value storage.
   * @param initial_value Starting value
   * @param min_value Minimum value (inclusive)
   * @param max_value Maximum value (inclusive)
   * @param step Increment/decrement step
   */
  SpinnerWidget(int32_t initial_value, int32_t min_value, int32_t max_value, int32_t step);

  ~SpinnerWidget() override;

  // Prevent copying
  SpinnerWidget(const SpinnerWidget&) = delete;
  SpinnerWidget& operator=(const SpinnerWidget&) = delete;

  // --- Event Subscription ---

  /**
   * @brief Set callback for when value changes during editing.
   */
  void set_on_changed(SpinnerChangeCallback callback);

  /**
   * @brief Set callback for when editing is committed.
   */
  void set_on_commit(SpinnerCommitCallback callback);

  /**
   * @brief Set callback to format display text.
   * If not set, displays raw integer value.
   * Note: Format should return ONLY the value portion, not the label.
   */
  void set_format(SpinnerFormatCallback callback);

  /**
   * @brief Set static label prefix (e.g., "Sleep: ").
   * This text appears before the value and is not highlighted in edit mode.
   * @param label Label text (must remain valid for widget lifetime)
   */
  void set_label(const char* label);

  // --- Preset Values ---

  /**
   * @brief Set discrete preset values instead of min/max/step.
   * @param presets Array of values to cycle through (must remain valid)
   * @param count Number of presets
   */
  void set_presets(const int32_t* presets, size_t count);

  // --- Accessors ---

  /**
   * @brief Get current value.
   */
  int32_t get_value() const;

  /**
   * @brief Set value directly.
   */
  void set_value(int32_t value);

  /**
   * @brief Check if currently in edit mode.
   */
  bool is_editing() const { return editing_; }

  /**
   * @brief Get minimum value.
   */
  int32_t get_min() const { return min_value_; }

  /**
   * @brief Get maximum value.
   */
  int32_t get_max() const { return max_value_; }

  /**
   * @brief Get step size.
   */
  int32_t get_step() const { return step_; }

  // --- Widget Interface ---

  void render(lv_obj_t* parent, int32_t y_offset) override;
  void destroy() override;
  void set_focused(bool focused) override;
  bool is_focused() const override { return focused_; }
  InputResult handle_input(bool short_press) override;

 private:
  // Value storage - either external pointer or internal
  int32_t* value_ptr_;
  int32_t internal_value_;
  bool owns_value_;  // true if using internal_value_

  int32_t min_value_;
  int32_t max_value_;
  int32_t step_;

  const int32_t* presets_ = nullptr;
  size_t preset_count_ = 0;
  size_t preset_index_ = 0;

  const char* label_ = nullptr;       // Static prefix text (e.g., "Sleep: ")
  lv_obj_t* lv_row_ = nullptr;        // Container for label + value
  lv_obj_t* lv_label_prefix_ = nullptr;  // Prefix label
  lv_obj_t* lv_label_value_ = nullptr;   // Value label (highlighted in edit mode)
  bool focused_ = false;
  bool editing_ = false;

  SpinnerChangeCallback on_changed_;
  SpinnerCommitCallback on_commit_;
  SpinnerFormatCallback format_;

  /**
   * @brief Cycle to next value.
   */
  void cycle_value();

  /**
   * @brief Update display text.
   */
  void update_display();

  /**
   * @brief Update visual appearance based on focus/edit state.
   */
  void update_appearance();

  /**
   * @brief Find preset index matching current value.
   */
  void sync_preset_index();

  /**
   * @brief Get pointer to current value (external or internal).
   */
  int32_t* value_storage();
};

}  // namespace ui
}  // namespace ezmodes
