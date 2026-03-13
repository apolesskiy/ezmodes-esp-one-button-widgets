/**
 * @file menu_controller.cpp
 * @brief Implementation of MenuController.
 */

#include "ezmodes/ui/menu_controller.hpp"

#include "ezmodes/ui/widgets/submenu_widget.hpp"
#include "esp_lvgl_port.h"

namespace ezmodes {
namespace ui {

MenuController::MenuController(lv_obj_t* screen) : screen_(screen) {}

MenuController::~MenuController() {
  clear();
}

void MenuController::push(Menu* menu) {
  if (menu == nullptr) {
    return;
  }

  if (!lvgl_port_lock(100)) {
    return;
  }

  // Save current focus state if there's an active menu
  if (!stack_.empty()) {
    stack_.back().focus_index = focus_index_;
    // Destroy current menu's LVGL objects
    current_menu()->destroy();
  }

  // Push new menu
  stack_.push_back({menu, 0});
  focus_index_ = 0;

  // Render the new menu
  render_current();

  // Focus the first focusable widget
  if (menu->focusable_count() > 0) {
    Widget* first = menu->get_focusable_child(0);
    if (first != nullptr) {
      first->set_focused(true);
    }
  }

  lvgl_port_unlock();
}

void MenuController::push_menu(std::unique_ptr<Menu> menu) {
  if (menu == nullptr) {
    return;
  }

  // Store ownership and push raw pointer
  Menu* menu_ptr = menu.get();
  owned_menus_.push_back(std::move(menu));
  push(menu_ptr);
}

bool MenuController::pop() {
  if (stack_.size() <= 1) {
    return false;  // Can't pop root or empty stack
  }

  if (!lvgl_port_lock(100)) {
    return false;
  }

  // Destroy current menu's LVGL objects
  current_menu()->destroy();

  // Pop current
  stack_.pop_back();

  // Restore previous menu
  StackFrame& frame = stack_.back();
  focus_index_ = frame.focus_index;

  // Render the restored menu
  render_current();

  // Restore focus
  if (frame.menu->focusable_count() > 0) {
    Widget* focused = frame.menu->get_focusable_child(focus_index_);
    if (focused != nullptr) {
      focused->set_focused(true);
    }
  }

  lvgl_port_unlock();
  return true;
}

void MenuController::clear() {
  // Destroy LVGL objects for current menu if any
  if (!stack_.empty()) {
    if (lvgl_port_lock(100)) {
      Menu* menu = current_menu();
      if (menu != nullptr) {
        menu->destroy();
      }
      lvgl_port_unlock();
    }
  }

  // Clear navigation state
  stack_.clear();
  focus_index_ = 0;

  // Release owned menus
  owned_menus_.clear();
}

Menu* MenuController::current_menu() const {
  if (stack_.empty()) {
    return nullptr;
  }
  return stack_.back().menu;
}

Widget* MenuController::get_focused_widget() const {
  Menu* menu = current_menu();
  if (menu == nullptr) {
    return nullptr;
  }
  return menu->get_focusable_child(focus_index_);
}

void MenuController::focus_next() {
  Menu* menu = current_menu();
  if (menu == nullptr) {
    return;
  }

  size_t count = menu->focusable_count();
  if (count == 0) {
    return;
  }

  size_t old_index = focus_index_;
  size_t new_index = (focus_index_ + 1) % count;

  update_focus_visuals(old_index, new_index);
  focus_index_ = new_index;

  // Scroll to keep focused widget visible
  // Find the actual child index for scrolling
  size_t child_index = 0;
  size_t focusable_seen = 0;
  for (size_t i = 0; i < menu->child_count(); i++) {
    Widget* child = menu->get_child(i);
    if (child != nullptr && child->is_focusable()) {
      if (focusable_seen == new_index) {
        child_index = i;
        break;
      }
      focusable_seen++;
    }
  }
  menu->scroll_to_child(child_index);
}

void MenuController::focus_prev() {
  Menu* menu = current_menu();
  if (menu == nullptr) {
    return;
  }

  size_t count = menu->focusable_count();
  if (count == 0) {
    return;
  }

  size_t old_index = focus_index_;
  size_t new_index = (focus_index_ == 0) ? (count - 1) : (focus_index_ - 1);

  update_focus_visuals(old_index, new_index);
  focus_index_ = new_index;
}

void MenuController::set_focus(size_t index) {
  Menu* menu = current_menu();
  if (menu == nullptr) {
    return;
  }

  size_t count = menu->focusable_count();
  if (index >= count) {
    return;
  }

  size_t old_index = focus_index_;
  update_focus_visuals(old_index, index);
  focus_index_ = index;
}

bool MenuController::handle_input(bool short_press) {
  Widget* focused = get_focused_widget();
  if (focused == nullptr) {
    return false;
  }

  if (!lvgl_port_lock(100)) {
    return false;
  }

  InputResult result = focused->handle_input(short_press);
  handle_input_result(result, focused);

  lvgl_port_unlock();
  return result != InputResult::kNotHandled;
}

void MenuController::render_current() {
  Menu* menu = current_menu();
  if (menu == nullptr || screen_ == nullptr) {
    return;
  }

  menu->render(screen_, 0);
}

void MenuController::update_focus_visuals(size_t old_index, size_t new_index) {
  Menu* menu = current_menu();
  if (menu == nullptr) {
    return;
  }

  if (old_index != new_index) {
    Widget* old_widget = menu->get_focusable_child(old_index);
    if (old_widget != nullptr) {
      old_widget->set_focused(false);
    }
  }

  Widget* new_widget = menu->get_focusable_child(new_index);
  if (new_widget != nullptr) {
    new_widget->set_focused(true);
  }
}

void MenuController::handle_input_result(InputResult result, Widget* source) {
  switch (result) {
    case InputResult::kFocusNext:
      focus_next();
      break;

    case InputResult::kFocusPrev:
      focus_prev();
      break;

    case InputResult::kSelected: {
      // Check if it's a submenu widget
      SubmenuWidget* submenu = static_cast<SubmenuWidget*>(source);
      if (submenu != nullptr) {
        push(submenu->get_submenu());
      }
      // Otherwise, the widget's callback handles the action
      break;
    }

    case InputResult::kDeselected:
    case InputResult::kConsumed:
    case InputResult::kNotHandled:
      // No controller action needed
      break;
  }
}

}  // namespace ui
}  // namespace ezmodes
