/**
 * @file info_screen_widget.cpp
 * @brief Implementation of InfoScreenWidget.
 */

#include "ezmodes/ui/widgets/info_screen_widget.hpp"

#include "esp_lvgl_port.h"

#include <cstring>

namespace ezmodes {
namespace ui {

InfoScreenWidget::InfoScreenWidget(const lv_font_t* display_font)
    : display_font_(display_font) {
  memset(lines_, 0, sizeof(lines_));
}

InfoScreenWidget::~InfoScreenWidget() {
  destroy();
}

void InfoScreenWidget::set_line(size_t index, const char* text) {
  if (index >= kMaxLines) {
    return;
  }
  if (text != nullptr) {
    snprintf(lines_[index], sizeof(lines_[index]), "%s", text);
  } else {
    lines_[index][0] = '\0';
  }
  if (index >= line_count_) {
    line_count_ = index + 1;
  }
  // Update LVGL label if already rendered
  if (lv_labels_[index] != nullptr) {
    lv_label_set_text(lv_labels_[index], lines_[index]);
  }
}

void InfoScreenWidget::set_on_back(InfoScreenBackCallback callback) {
  on_back_ = std::move(callback);
}

void InfoScreenWidget::render(lv_obj_t* parent, int32_t y_offset) {
  destroy();

  int32_t line_h = display_font_ ? display_font_->line_height : 6;
  int32_t y = y_offset;

  for (size_t i = 0; i < line_count_; i++) {
    lv_labels_[i] = lv_label_create(parent);
    lv_label_set_text(lv_labels_[i], lines_[i]);
    lv_label_set_long_mode(lv_labels_[i], LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(lv_labels_[i], 0, y);
    lv_obj_set_width(lv_labels_[i], lv_pct(100));
    if (display_font_ != nullptr) {
      lv_obj_set_style_text_font(lv_labels_[i], display_font_, 0);
    }
    lv_obj_set_style_pad_left(lv_labels_[i], 1, 0);
    lv_obj_set_style_bg_opa(lv_labels_[i], LV_OPA_TRANSP, 0);
    lv_obj_set_style_text_color(lv_labels_[i], lv_color_white(), 0);
    y += line_h;
  }
}

void InfoScreenWidget::destroy() {
  for (size_t i = 0; i < kMaxLines; i++) {
    if (lv_labels_[i] != nullptr) {
      if (lvgl_port_lock(1000)) {
        lv_obj_del(lv_labels_[i]);
        lvgl_port_unlock();
      }
      lv_labels_[i] = nullptr;
    }
  }
}

void InfoScreenWidget::set_focused(bool focused) {
  focused_ = focused;
}

InputResult InfoScreenWidget::handle_input(bool short_press) {
  if (short_press) {
    return InputResult::kConsumed;
  }
  // Long press → back
  if (on_back_) {
    on_back_(*this);
  }
  return InputResult::kSelected;
}

int32_t InfoScreenWidget::get_height() const {
  int32_t line_h = display_font_ ? display_font_->line_height : 6;
  return static_cast<int32_t>(line_count_) * line_h;
}

}  // namespace ui
}  // namespace ezmodes
