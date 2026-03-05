/**
 * @file toggle_widget.cpp
 * @brief Implementation of ToggleWidget.
 */

#include "ezmodes/ui/widgets/toggle_widget.hpp"

#include <cstdio>

#include "esp_lvgl_port.h"

namespace ezmodes {
namespace ui {

ToggleWidget::ToggleWidget(const char* label, bool* value_ptr)
    : label_(label),
      value_ptr_(value_ptr),
      internal_value_(false),
      owns_value_(false) {}

ToggleWidget::ToggleWidget(const char* label, bool initial_value)
    : label_(label),
      value_ptr_(nullptr),
      internal_value_(initial_value),
      owns_value_(true) {}

ToggleWidget::~ToggleWidget() {
  destroy();
}

void ToggleWidget::set_on_changed(ToggleCallback callback) {
  on_changed_ = std::move(callback);
}

bool ToggleWidget::get_value() const {
  if (owns_value_) {
    return internal_value_;
  }
  return value_ptr_ ? *value_ptr_ : false;
}

void ToggleWidget::set_value(bool value) {
  if (owns_value_) {
    internal_value_ = value;
  } else if (value_ptr_) {
    *value_ptr_ = value;
  }
  update_display();
}

void ToggleWidget::set_state_text(const char* on_text, const char* off_text) {
  on_text_ = on_text;
  off_text_ = off_text;
  update_display();
}

bool* ToggleWidget::value_storage() {
  return owns_value_ ? &internal_value_ : value_ptr_;
}

void ToggleWidget::render(lv_obj_t* parent, int32_t y_offset) {
  destroy();

  lv_label_ = lv_label_create(parent);
  lv_label_set_long_mode(lv_label_, LV_LABEL_LONG_DOT);
  lv_obj_set_pos(lv_label_, 0, y_offset);
  lv_obj_set_width(lv_label_, lv_pct(100));
  if (font_) {
    lv_obj_set_style_text_font(lv_label_, font_, 0);
  }
  lv_obj_set_style_pad_left(lv_label_, 2, 0);

  update_display();
  update_appearance();
}

void ToggleWidget::destroy() {
  if (lv_label_ != nullptr) {
    if (lvgl_port_lock(1000)) {
      lv_obj_del(lv_label_);
      lvgl_port_unlock();
    }
    lv_label_ = nullptr;
  }
}

void ToggleWidget::set_focused(bool focused) {
  if (focused_ != focused) {
    focused_ = focused;
    update_appearance();
  }
}

InputResult ToggleWidget::handle_input(bool short_press) {
  if (short_press) {
    // Short press moves to next item
    return InputResult::kFocusNext;
  } else {
    // Long press toggles value
    bool* storage = value_storage();
    if (storage != nullptr) {
      *storage = !*storage;
      update_display();
      if (on_changed_) {
        on_changed_(*this, get_value());
      }
    }
    return InputResult::kConsumed;
  }
}

void ToggleWidget::update_display() {
  if (lv_label_ == nullptr) {
    return;
  }

  char buffer[64];
  const char* state = get_value() ? on_text_ : off_text_;
  snprintf(buffer, sizeof(buffer), "%s: %s", label_, state);
  lv_label_set_text(lv_label_, buffer);
}

void ToggleWidget::update_appearance() {
  if (lv_label_ == nullptr) {
    return;
  }

  if (focused_) {
    // Highlighted: white background, black text
    lv_obj_set_style_bg_color(lv_label_, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(lv_label_, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(lv_label_, lv_color_black(), 0);
  } else {
    // Normal: transparent background, white text
    lv_obj_set_style_bg_opa(lv_label_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_text_color(lv_label_, lv_color_white(), 0);
  }
}

}  // namespace ui
}  // namespace ezmodes
