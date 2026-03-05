#pragma once

/**
 * @file menu_controller.hpp
 * @brief Controller that manages menu navigation and input routing.
 *
 * The MenuController maintains a stack of Menu containers for nested
 * navigation, tracks the currently focused widget, and routes button
 * input to the focused widget.
 */

#include "ezmodes/ui/menu.hpp"

#include <memory>
#include <vector>

namespace ezmodes {
namespace ui {

// Forward declaration
class SubmenuWidget;

/**
 * @brief Controller that manages menu navigation and input routing.
 *
 * Responsibilities:
 * - Maintains a stack of Menu containers for nested navigation
 * - Tracks the cursor (currently focused widget index within active menu)
 * - Routes button input to the focused widget
 * - Handles InputResult to manage focus transitions and navigation
 *
 * The controller does NOT own the widgets - it manages navigation through
 * a widget tree that the consumer builds and owns.
 */
class MenuController {
 public:
  /**
   * @brief Construct controller for given screen.
   * @param screen LVGL screen to render menus on
   */
  explicit MenuController(lv_obj_t* screen);

  ~MenuController();

  // Prevent copying
  MenuController(const MenuController&) = delete;
  MenuController& operator=(const MenuController&) = delete;

  // --- Menu Stack Management ---

  /**
   * @brief Push a menu onto the stack and display it.
   * @param menu Menu to display (controller does not take ownership)
   */
  void push(Menu* menu);

  /**
   * @brief Push an owned menu onto the stack and display it.
   *
   * Use this for menus created by services where the controller should
   * take ownership. The menu will be destroyed when popped or cleared.
   *
   * @param menu Menu to display (controller takes ownership)
   */
  void push_menu(std::unique_ptr<Menu> menu);

  /**
   * @brief Pop the current menu and return to previous.
   * @return true if popped, false if already at root
   */
  bool pop();

  /**
   * @brief Clear the entire stack and hide all menus.
   */
  void clear();

  /**
   * @brief Check if any menu is currently displayed.
   */
  bool is_active() const { return !stack_.empty(); }

  /**
   * @brief Get the currently active menu (top of stack).
   * @return Active menu, or nullptr if stack is empty
   */
  Menu* current_menu() const;

  /**
   * @brief Get the current stack depth.
   */
  size_t stack_depth() const { return stack_.size(); }

  // --- Focus Management ---

  /**
   * @brief Get index of currently focused widget in active menu.
   */
  size_t get_focus_index() const { return focus_index_; }

  /**
   * @brief Get the currently focused widget.
   * @return Focused widget, or nullptr if none
   */
  Widget* get_focused_widget() const;

  /**
   * @brief Move focus to next focusable widget.
   */
  void focus_next();

  /**
   * @brief Move focus to previous focusable widget.
   */
  void focus_prev();

  /**
   * @brief Set focus to specific widget index (among focusable widgets).
   * @param index Index among focusable widgets
   */
  void set_focus(size_t index);

  // --- Input Routing ---

  /**
   * @brief Route button input to the focused widget.
   * @param short_press true for short press, false for long press
   * @return true if input was consumed
   */
  bool handle_input(bool short_press);

 private:
  /**
   * @brief Stack frame storing menu and its saved focus state.
   */
  struct StackFrame {
    Menu* menu;
    size_t focus_index;
  };

  lv_obj_t* screen_;
  std::vector<StackFrame> stack_;
  std::vector<std::unique_ptr<Menu>> owned_menus_;  ///< Menus we own
  size_t focus_index_ = 0;

  /**
   * @brief Render the current menu.
   */
  void render_current();

  /**
   * @brief Update focus visuals when focus changes.
   */
  void update_focus_visuals(size_t old_index, size_t new_index);

  /**
   * @brief Handle the result from widget input processing.
   * @param result The InputResult from the widget
   * @param source The widget that produced the result
   */
  void handle_input_result(InputResult result, Widget* source);
};

}  // namespace ui
}  // namespace ezmodes
