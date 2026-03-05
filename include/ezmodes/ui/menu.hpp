#pragma once

/**
 * @file menu.hpp
 * @brief Container widget that holds a vertical list of child widgets.
 *
 * Menus are the primary container type for the menu system. They can be
 * nested (submenu pattern) and manage scrolling of their children.
 */

#include "ezmodes/ui/widget.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace ezmodes {
namespace ui {

/**
 * @brief Container widget that holds a vertical list of child widgets.
 *
 * Menus are the primary container type. They can be nested (submenu pattern).
 * The MenuController pushes/pops menus onto its stack for navigation.
 */
class Menu : public Widget {
 public:
  /**
   * @brief Construct a menu.
   * @param title Optional title displayed at top (nullptr for no title)
   * @param config Font/layout configuration propagated to child widgets
   */
  explicit Menu(const char* title = nullptr,
                const MenuConfig& config = MenuConfig{});

  ~Menu() override;

  // Prevent copying
  Menu(const Menu&) = delete;
  Menu& operator=(const Menu&) = delete;

  // Allow moving
  Menu(Menu&&) = default;
  Menu& operator=(Menu&&) = default;

  // --- Child Management ---

  /**
   * @brief Add a child widget to this menu.
   * @param widget Unique pointer to widget (menu takes ownership)
   */
  void add_child(std::unique_ptr<Widget> widget);

  /**
   * @brief Get number of children.
   */
  size_t child_count() const { return children_.size(); }

  /**
   * @brief Get number of focusable children.
   */
  size_t focusable_count() const;

  /**
   * @brief Get child widget by index.
   * @param index Zero-based index
   * @return Pointer to child widget, or nullptr if out of range
   */
  Widget* get_child(size_t index);

  /**
   * @brief Get child widget by index (const).
   * @param index Zero-based index
   * @return Pointer to child widget, or nullptr if out of range
   */
  const Widget* get_child(size_t index) const;

  /**
   * @brief Find the nth focusable child.
   * @param focusable_index Index among focusable children only
   * @return Pointer to widget, or nullptr if out of range
   */
  Widget* get_focusable_child(size_t focusable_index);

  /**
   * @brief Get the focusable index of a child.
   * @param child_index Index in the full children list
   * @return Index among focusable children, or -1 if not focusable
   */
  int get_focusable_index(size_t child_index) const;

  /**
   * @brief Get menu title.
   */
  const char* get_title() const { return title_; }

  /**
   * @brief Get the menu configuration.
   */
  const MenuConfig& get_config() const { return config_; }

  // --- Widget Interface ---

  void render(lv_obj_t* parent, int32_t y_offset) override;
  void destroy() override;
  void set_focused(bool focused) override;
  bool is_focused() const override { return false; }  // Container itself not focused
  bool is_focusable() const override { return false; }  // Container, not focusable
  InputResult handle_input(bool short_press) override;
  int32_t get_height() const override;

  // --- Scrolling ---

  /**
   * @brief Scroll to ensure the widget at given index is visible.
   * @param index Index of child to scroll to
   */
  void scroll_to_child(size_t index);

 private:
  const char* title_;
  MenuConfig config_;
  std::vector<std::unique_ptr<Widget>> children_;
  lv_obj_t* container_ = nullptr;
};

}  // namespace ui
}  // namespace ezmodes
