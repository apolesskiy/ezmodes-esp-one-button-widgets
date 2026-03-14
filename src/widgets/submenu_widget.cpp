/**
 * @file submenu_widget.cpp
 * @brief Implementation of SubmenuWidget.
 */

#include "ezmodes/ui/widgets/submenu_widget.hpp"

#include "esp_lvgl_port.h"

namespace ezmodes {
namespace ui {

SubmenuWidget::SubmenuWidget(const char* label, std::unique_ptr<Menu> submenu)
    : label_(label), submenu_(std::move(submenu)) {}

SubmenuWidget::~SubmenuWidget() {
  destroy();
}

void SubmenuWidget::render(lv_obj_t* parent, int32_t y_offset) {
  // Clean up existing
  destroy();

  // Create label with indicator
  lv_label_ = lv_label_create(parent);

  // Format as "label"
  static char buffer[64];
  snprintf(buffer, sizeof(buffer), "%s", label_);
  lv_label_set_text(lv_label_, buffer);
  lv_label_set_long_mode(lv_label_, LV_LABEL_LONG_DOT);

  lv_obj_set_pos(lv_label_, 0, y_offset);
  lv_obj_set_width(lv_label_, lv_pct(100));
  if (font_) {
    lv_obj_set_style_text_font(lv_label_, font_, 0);
  }
  lv_obj_set_style_pad_left(lv_label_, 2, 0);

  update_appearance();
}

void SubmenuWidget::destroy() {
  if (lv_label_ != nullptr) {
    if (lvgl_port_lock(1000)) {
      lv_obj_del(lv_label_);
      lvgl_port_unlock();
    }
    lv_label_ = nullptr;
  }
}

void SubmenuWidget::set_focused(bool focused) {
  if (focused_ != focused) {
    focused_ = focused;
    update_appearance();
  }
}

InputResult SubmenuWidget::handle_input(bool short_press) {
  if (short_press) {
    // Short press moves to next item
    return InputResult::kFocusNext;
  } else {
    // Long press indicates selection - controller will push submenu
    return InputResult::kSelected;
  }
}

void SubmenuWidget::update_appearance() {
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
