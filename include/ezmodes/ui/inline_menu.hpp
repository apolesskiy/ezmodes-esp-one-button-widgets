#pragma once

/**
 * @file inline_menu.hpp
 * @brief Reusable scrollable menu widget with inline value editing.
 *
 * Provides a simple vertical menu that can be navigated with a single button.
 * Supports inline editing of values with spinner-style cycling.
 */

#include "esp_lvgl_port.h"
#include "lvgl.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace ezmodes {
namespace ui {

/**
 * @brief A single menu item that can be displayed and optionally edited.
 */
struct MenuItem {
  int id;               ///< User-defined identifier for the item
  const char* text;     ///< Display text (must remain valid while in menu)
  bool editable;        ///< Whether item can enter edit mode

  MenuItem(int id, const char* text, bool editable = false)
      : id(id), text(text), editable(editable) {}
};

/**
 * @brief Callback when a menu item is selected (long press).
 * @param item_id The id of the selected item
 */
using OnSelectCallback = std::function<void(int item_id)>;

/**
 * @brief Callback when an editable value should be cycled.
 * @param item_id The id of the item being edited
 * @return New text to display for the item
 */
using OnCycleCallback = std::function<const char*(int item_id)>;

/**
 * @brief Callback when edit mode is exited for an item.
 * @param item_id The id of the item that was edited
 */
using OnEditDoneCallback = std::function<void(int item_id)>;

/**
 * @brief Scrollable menu widget with inline editing support.
 *
 * Usage:
 * 1. Create menu with parent container
 * 2. Add items with add_item()
 * 3. Call set_callbacks() to register handlers
 * 4. Call handle_button() from mode's button handler
 */
class InlineMenu {
 public:
  /**
   * @brief Construct menu on given parent container.
   * @param parent LVGL object to contain the menu
   * @param font Font used for item labels (nullptr uses LVGL default)
   * @param height Visible height in pixels
   * @param item_height Height of each menu item
   */
  InlineMenu(lv_obj_t* parent, const lv_font_t* font = nullptr,
             int32_t height = 48, int32_t item_height = 12);

  /**
   * @brief Destructor - cleans up LVGL objects.
   */
  ~InlineMenu();

  /**
   * @brief Add an item to the menu.
   * @param id User-defined identifier
   * @param text Display text
   * @param editable Whether the item supports inline editing
   */
  void add_item(int id, const char* text, bool editable = false);

  /**
   * @brief Clear all items from the menu.
   */
  void clear();

  /**
   * @brief Set callback handlers.
   * @param on_select Called when item is selected (long press on non-editable)
   * @param on_cycle Called when editable value should cycle
   * @param on_edit_done Called when exiting edit mode
   */
  void set_callbacks(OnSelectCallback on_select,
                     OnCycleCallback on_cycle = nullptr,
                     OnEditDoneCallback on_edit_done = nullptr);

  /**
   * @brief Handle button event from mode.
   * @param short_press True for short press, false for long press
   * @return True if event was handled
   */
  bool handle_button(bool short_press);

  /**
   * @brief Check if menu is currently in edit mode.
   */
  bool is_editing() const { return editing_index_ >= 0; }

  /**
   * @brief Get currently selected item index.
   */
  size_t get_selected_index() const { return selected_index_; }

  /**
   * @brief Get ID of currently selected item.
   */
  int get_selected_id() const;

  /**
   * @brief Update text of a specific item by id.
   * @param id Item identifier
   * @param text New text to display
   */
  void update_item_text(int id, const char* text);

 private:
  /**
   * @brief Update visual highlight on selected item.
   */
  void update_highlight();

  /**
   * @brief Scroll to keep selected item visible.
   */
  void scroll_to_selection();

  /**
   * @brief Enter edit mode for current item.
   */
  void start_editing();

  /**
   * @brief Exit edit mode.
   */
  void stop_editing();

  lv_obj_t* container_;       ///< Scroll container
  const lv_font_t* font_;    ///< Font for item labels
  int32_t height_;            ///< Visible height
  int32_t item_height_;       ///< Height per item

  struct ItemData {
    lv_obj_t* label;
    int id;
    bool editable;
  };
  std::vector<ItemData> items_;
  size_t selected_index_ = 0;
  int editing_index_ = -1;

  OnSelectCallback on_select_;
  OnCycleCallback on_cycle_;
  OnEditDoneCallback on_edit_done_;
};

}  // namespace ui
}  // namespace ezmodes
