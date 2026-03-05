/**
 * @file menu.cpp
 * @brief Implementation of Menu container widget.
 */

#include "ezmodes/ui/menu.hpp"

#include "esp_lvgl_port.h"

namespace ezmodes {
namespace ui {

Menu::Menu(const char* title, const MenuConfig& config)
    : title_(title), config_(config) {}

Menu::~Menu() {
  destroy();
}

void Menu::add_child(std::unique_ptr<Widget> widget) {
  children_.push_back(std::move(widget));
}

size_t Menu::focusable_count() const {
  size_t count = 0;
  for (const auto& child : children_) {
    if (child->is_focusable()) {
      count++;
    }
  }
  return count;
}

Widget* Menu::get_child(size_t index) {
  if (index >= children_.size()) {
    return nullptr;
  }
  return children_[index].get();
}

const Widget* Menu::get_child(size_t index) const {
  if (index >= children_.size()) {
    return nullptr;
  }
  return children_[index].get();
}

Widget* Menu::get_focusable_child(size_t focusable_index) {
  size_t count = 0;
  for (auto& child : children_) {
    if (child->is_focusable()) {
      if (count == focusable_index) {
        return child.get();
      }
      count++;
    }
  }
  return nullptr;
}

int Menu::get_focusable_index(size_t child_index) const {
  if (child_index >= children_.size()) {
    return -1;
  }
  if (!children_[child_index]->is_focusable()) {
    return -1;
  }

  int focusable_index = 0;
  for (size_t i = 0; i < child_index; i++) {
    if (children_[i]->is_focusable()) {
      focusable_index++;
    }
  }
  return focusable_index;
}

void Menu::render(lv_obj_t* parent, int32_t y_offset) {
  // Clean up existing container
  destroy();

  // Create scrollable container constrained to parent's content area.
  // Using content height accounts for any padding on the parent (e.g. the
  // 8-px header bar pad_top on the screen object).
  container_ = lv_obj_create(parent);
  lv_obj_remove_style_all(container_);  // Strip default theme styles
  int32_t parent_h = lv_obj_get_content_height(parent);
  int32_t avail_h = parent_h - y_offset;
  if (avail_h < 1) {
    avail_h = parent_h;
  }
  lv_obj_set_size(container_, lv_pct(100), avail_h);
  lv_obj_set_pos(container_, 0, y_offset);
  lv_obj_set_style_pad_all(container_, 0, 0);
  lv_obj_set_style_border_width(container_, 0, 0);
  lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
  lv_obj_set_scrollbar_mode(container_, LV_SCROLLBAR_MODE_OFF);
  lv_obj_add_flag(container_, LV_OBJ_FLAG_SCROLLABLE);

  int32_t current_y = 0;

  // Propagate font to each child, then render.
  for (auto& child : children_) {
    child->set_font(config_.font_large);
    child->render(container_, current_y);
    current_y += child->get_height();
  }
}

void Menu::destroy() {
  // Children will clean up their own LVGL objects when we delete the container
  if (container_ != nullptr) {
    // LVGL operations require the port lock. The lock is recursive, so nested
    // calls (e.g., from child widget destructors) are safe.
    if (lvgl_port_lock(1000)) {
      // First destroy children so they clean up their references
      for (auto& child : children_) {
        child->destroy();
      }
      lv_obj_del(container_);
      lvgl_port_unlock();
    }
    container_ = nullptr;
  }
}

void Menu::set_focused(bool focused) {
  // Container itself doesn't show focus, this is a no-op
  (void)focused;
}

InputResult Menu::handle_input(bool short_press) {
  // Container doesn't handle input directly - controller routes to children
  (void)short_press;
  return InputResult::kNotHandled;
}

int32_t Menu::get_height() const {
  int32_t total = 0;
  for (const auto& child : children_) {
    total += child->get_height();
  }
  return total;
}

void Menu::scroll_to_child(size_t index) {
  if (container_ == nullptr || index >= children_.size()) {
    return;
  }

  // Use the child's LVGL object to scroll it into view
  Widget* child = children_[index].get();
  if (child == nullptr) {
    return;
  }

  // Calculate y position of the target child
  int32_t target_y = 0;
  for (size_t i = 0; i < index; i++) {
    target_y += children_[i]->get_height();
  }

  int32_t child_h = children_[index]->get_height();
  int32_t container_h = lv_obj_get_height(container_);
  int32_t current_scroll = lv_obj_get_scroll_y(container_);

  // Check if child is below the visible area
  if (target_y + child_h > current_scroll + container_h) {
    lv_obj_scroll_to_y(container_, target_y + child_h - container_h,
                       LV_ANIM_OFF);
  }
  // Check if child is above the visible area
  else if (target_y < current_scroll) {
    // Scroll to show title when targeting the first child
    int32_t scroll_target = (index == 0) ? 0 : target_y;
    lv_obj_scroll_to_y(container_, scroll_target, LV_ANIM_OFF);
  }
}

}  // namespace ui
}  // namespace ezmodes
