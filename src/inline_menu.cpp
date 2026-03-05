/**
 * @file inline_menu.cpp
 * @brief Implementation of reusable inline menu widget.
 */

#include "ezmodes/ui/inline_menu.hpp"

namespace ezmodes {
namespace ui {

InlineMenu::InlineMenu(lv_obj_t* parent, const lv_font_t* font,
                       int32_t height, int32_t item_height)
    : font_(font), height_(height), item_height_(item_height) {
  // Create scrollable container
  container_ = lv_obj_create(parent);
  lv_obj_set_size(container_, 64, height_);
  lv_obj_align(container_, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_set_style_bg_color(container_, lv_color_black(), 0);
  lv_obj_set_style_border_width(container_, 0, 0);
  lv_obj_set_style_pad_all(container_, 0, 0);
  lv_obj_set_scrollbar_mode(container_, LV_SCROLLBAR_MODE_OFF);
}

InlineMenu::~InlineMenu() {
  if (container_ != nullptr) {
    lv_obj_del(container_);
    container_ = nullptr;
  }
}

void InlineMenu::add_item(int id, const char* text, bool editable) {
  lv_obj_t* label = lv_label_create(container_);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_color(label, lv_color_white(), 0);
  if (font_ != nullptr) {
    lv_obj_set_style_text_font(label, font_, 0);
  }
  lv_obj_set_pos(label, 2, items_.size() * item_height_ + 1);

  items_.push_back({label, id, editable});

  // Update highlight if this is the first item
  if (items_.size() == 1) {
    update_highlight();
  }
}

void InlineMenu::clear() {
  for (auto& item : items_) {
    if (item.label != nullptr) {
      lv_obj_del(item.label);
    }
  }
  items_.clear();
  selected_index_ = 0;
  editing_index_ = -1;
}

void InlineMenu::set_callbacks(OnSelectCallback on_select,
                               OnCycleCallback on_cycle,
                               OnEditDoneCallback on_edit_done) {
  on_select_ = std::move(on_select);
  on_cycle_ = std::move(on_cycle);
  on_edit_done_ = std::move(on_edit_done);
}

bool InlineMenu::handle_button(bool short_press) {
  if (items_.empty()) {
    return false;
  }

  if (editing_index_ >= 0) {
    // In edit mode
    if (short_press) {
      // Cycle the value
      if (on_cycle_) {
        const char* new_text = on_cycle_(items_[editing_index_].id);
        if (new_text != nullptr && lvgl_port_lock(100)) {
          lv_label_set_text(items_[editing_index_].label, new_text);
          lvgl_port_unlock();
        }
      }
    } else {
      // Exit edit mode
      stop_editing();
    }
  } else {
    // Normal navigation mode
    if (short_press) {
      // Move to next item
      selected_index_ = (selected_index_ + 1) % items_.size();
      if (lvgl_port_lock(100)) {
        update_highlight();
        scroll_to_selection();
        lvgl_port_unlock();
      }
    } else {
      // Select/enter edit mode
      if (items_[selected_index_].editable) {
        start_editing();
      } else if (on_select_) {
        on_select_(items_[selected_index_].id);
      }
    }
  }

  return true;
}

int InlineMenu::get_selected_id() const {
  if (selected_index_ < items_.size()) {
    return items_[selected_index_].id;
  }
  return -1;
}

void InlineMenu::update_item_text(int id, const char* text) {
  for (auto& item : items_) {
    if (item.id == id && item.label != nullptr) {
      if (lvgl_port_lock(100)) {
        lv_label_set_text(item.label, text);
        lvgl_port_unlock();
      }
      break;
    }
  }
}

void InlineMenu::update_highlight() {
  for (size_t i = 0; i < items_.size(); i++) {
    if (items_[i].label == nullptr) continue;

    if (i == selected_index_) {
      // Highlight selected (inverted colors)
      lv_obj_set_style_bg_color(items_[i].label, lv_color_white(), 0);
      lv_obj_set_style_bg_opa(items_[i].label, LV_OPA_COVER, 0);
      lv_obj_set_style_text_color(items_[i].label, lv_color_black(), 0);
    } else {
      // Normal colors
      lv_obj_set_style_bg_opa(items_[i].label, LV_OPA_TRANSP, 0);
      lv_obj_set_style_text_color(items_[i].label, lv_color_white(), 0);
    }
  }
}

void InlineMenu::scroll_to_selection() {
  if (container_ == nullptr) return;

  int32_t item_y = selected_index_ * item_height_;
  int32_t scroll_y = lv_obj_get_scroll_y(container_);

  if (item_y < scroll_y) {
    lv_obj_scroll_to_y(container_, item_y, LV_ANIM_OFF);
  } else if (item_y + item_height_ > scroll_y + height_) {
    lv_obj_scroll_to_y(container_, item_y + item_height_ - height_,
                       LV_ANIM_OFF);
  }
}

void InlineMenu::start_editing() {
  editing_index_ = static_cast<int>(selected_index_);
}

void InlineMenu::stop_editing() {
  int edited_id = items_[editing_index_].id;
  editing_index_ = -1;

  if (on_edit_done_) {
    on_edit_done_(edited_id);
  }
}

}  // namespace ui
}  // namespace ezmodes
