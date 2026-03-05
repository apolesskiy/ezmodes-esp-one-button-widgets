/**
 * @file action_widget.cpp
 * @brief Implementation of ActionWidget.
 */

#include "ezmodes/ui/widgets/action_widget.hpp"

#include "esp_lvgl_port.h"

namespace ezmodes {
namespace ui {

ActionWidget::ActionWidget(const char* label)
    : label_storage_(label ? label : ""), label_(label_storage_.c_str()) {}

ActionWidget::~ActionWidget() {
  destroy();
}

void ActionWidget::set_on_selected(ActionCallback callback) {
  on_selected_ = std::move(callback);
}

void ActionWidget::set_label(const char* label) {
  label_storage_ = label ? label : "";
  label_ = label_storage_.c_str();
  if (lv_label_ != nullptr) {
    lv_label_set_text(lv_label_, label_);
  }
}

void ActionWidget::render(lv_obj_t* parent, int32_t y_offset) {
  // Clean up existing
  destroy();

  // Create label
  lv_label_ = lv_label_create(parent);
  lv_label_set_text(lv_label_, label_);
  lv_label_set_long_mode(lv_label_, LV_LABEL_LONG_DOT);
  lv_obj_set_pos(lv_label_, 0, y_offset);
  lv_obj_set_width(lv_label_, lv_pct(100));
  if (font_) {
    lv_obj_set_style_text_font(lv_label_, font_, 0);
  }
  lv_obj_set_style_pad_left(lv_label_, 2, 0);

  update_appearance();
}

void ActionWidget::destroy() {
  if (lv_label_ != nullptr) {
    if (lvgl_port_lock(1000)) {
      lv_obj_del(lv_label_);
      lvgl_port_unlock();
    }
    lv_label_ = nullptr;
  }
}

void ActionWidget::set_focused(bool focused) {
  if (focused_ != focused) {
    focused_ = focused;
    update_appearance();
  }
}

InputResult ActionWidget::handle_input(bool short_press) {
  if (short_press) {
    // Short press moves to next item
    return InputResult::kFocusNext;
  } else {
    // Long press selects this action
    if (on_selected_) {
      on_selected_(*this);
    }
    return InputResult::kSelected;
  }
}

void ActionWidget::update_appearance() {
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
