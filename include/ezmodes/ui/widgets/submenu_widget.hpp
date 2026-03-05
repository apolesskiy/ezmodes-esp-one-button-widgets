#pragma once

/**
 * @file submenu_widget.hpp
 * @brief Widget that opens a submenu when selected.
 *
 * Displays as a menu item with indicator (e.g., ">"). When selected,
 * it tells the MenuController to push the associated submenu.
 */

#include "ezmodes/ui/menu.hpp"
#include "ezmodes/ui/widget.hpp"

#include <memory>

namespace ezmodes {
namespace ui {

/**
 * @brief Widget that opens a submenu when selected.
 *
 * Displays as a menu item with indicator (e.g., ">"). When selected,
 * the MenuController detects this widget type and pushes the submenu.
 *
 * Input behavior:
 * - Short press: Returns kFocusNext (move to next item)
 * - Long press: Returns kSelected (controller pushes submenu)
 */
class SubmenuWidget : public Widget {
 public:
  /**
   * @brief Construct a submenu widget.
   * @param label Display text (must remain valid for widget lifetime)
   * @param submenu The menu to push when selected (ownership transferred)
   */
  SubmenuWidget(const char* label, std::unique_ptr<Menu> submenu);

  ~SubmenuWidget() override;

  // Prevent copying
  SubmenuWidget(const SubmenuWidget&) = delete;
  SubmenuWidget& operator=(const SubmenuWidget&) = delete;

  // --- Accessors ---

  /**
   * @brief Get the label text.
   */
  const char* get_label() const { return label_; }

  /**
   * @brief Get the submenu.
   */
  Menu* get_submenu() { return submenu_.get(); }

  /**
   * @brief Get the submenu (const).
   */
  const Menu* get_submenu() const { return submenu_.get(); }

  // --- Widget Interface ---

  void render(lv_obj_t* parent, int32_t y_offset) override;
  void destroy() override;
  void set_focused(bool focused) override;
  bool is_focused() const override { return focused_; }
  InputResult handle_input(bool short_press) override;

 private:
  const char* label_;
  std::unique_ptr<Menu> submenu_;
  lv_obj_t* lv_label_ = nullptr;
  bool focused_ = false;

  /**
   * @brief Update visual appearance based on focus state.
   */
  void update_appearance();
};

}  // namespace ui
}  // namespace ezmodes
