#pragma once

/**
 * @file action_widget.hpp
 * @brief Simple selectable action widget (button/menu item).
 *
 * Emits on_selected when long-pressed. Used for navigation items,
 * back buttons, and actions that trigger immediate effects.
 */

#include "ezmodes/ui/widget.hpp"

#include <functional>
#include <string>

namespace ezmodes {
namespace ui {

// Forward declaration for callback
class ActionWidget;

/**
 * @brief Callback when action widget is selected (long press).
 * @param widget Reference to the widget that was selected
 */
using ActionCallback = std::function<void(ActionWidget& widget)>;

/**
 * @brief Simple selectable action widget (button/menu item).
 *
 * Emits on_selected when long-pressed. Used for navigation items,
 * back buttons, and actions that trigger immediate effects.
 *
 * Input behavior:
 * - Short press: Returns kFocusNext (move to next item)
 * - Long press: Invokes on_selected callback, returns kSelected
 */
class ActionWidget : public Widget {
 public:
  /**
   * @brief Construct an action widget.
   * @param label Display text (must remain valid for widget lifetime)
   */
  explicit ActionWidget(const char* label);

  ~ActionWidget() override;

  // Prevent copying
  ActionWidget(const ActionWidget&) = delete;
  ActionWidget& operator=(const ActionWidget&) = delete;

  // --- Event Subscription ---

  /**
   * @brief Set callback for when this action is selected.
   * @param callback Function to call on selection
   */
  void set_on_selected(ActionCallback callback);

  // --- Accessors ---

  /**
   * @brief Get the current label text.
   */
  const char* get_label() const { return label_; }

  /**
   * @brief Set new label text.
   * @param label New text (must remain valid for widget lifetime)
   */
  void set_label(const char* label);

  // --- Widget Interface ---

  void render(lv_obj_t* parent, int32_t y_offset) override;
  void destroy() override;
  void set_focused(bool focused) override;
  bool is_focused() const override { return focused_; }
  InputResult handle_input(bool short_press) override;

 private:
  std::string label_storage_;
  const char* label_;
  lv_obj_t* lv_label_ = nullptr;
  bool focused_ = false;
  ActionCallback on_selected_;

  /**
   * @brief Update visual appearance based on focus state.
   */
  void update_appearance();
};

}  // namespace ui
}  // namespace ezmodes
