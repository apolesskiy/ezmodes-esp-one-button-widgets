/**
 * @file spinner_widget.cpp
 * @brief Implementation of SpinnerWidget.
 */

#include "ezmodes/ui/widgets/spinner_widget.hpp"

#include <cstdio>

#include "esp_lvgl_port.h"

namespace ezmodes {
namespace ui {

SpinnerWidget::SpinnerWidget(int32_t* value_ptr, int32_t min_value,
                             int32_t max_value, int32_t step)
    : value_ptr_(value_ptr),
      internal_value_(0),
      owns_value_(false),
      min_value_(min_value),
      max_value_(max_value),
      step_(step) {}

SpinnerWidget::SpinnerWidget(int32_t initial_value, int32_t min_value,
                             int32_t max_value, int32_t step)
    : value_ptr_(nullptr),
      internal_value_(initial_value),
      owns_value_(true),
      min_value_(min_value),
      max_value_(max_value),
      step_(step) {}

SpinnerWidget::~SpinnerWidget() {
  destroy();
}

void SpinnerWidget::set_on_changed(SpinnerChangeCallback callback) {
  on_changed_ = std::move(callback);
}

void SpinnerWidget::set_on_commit(SpinnerCommitCallback callback) {
  on_commit_ = std::move(callback);
}

void SpinnerWidget::set_format(SpinnerFormatCallback callback) {
  format_ = std::move(callback);
}

void SpinnerWidget::set_label(const char* label) {
  label_ = label;
}

void SpinnerWidget::set_presets(const int32_t* presets, size_t count) {
  presets_ = presets;
  preset_count_ = count;
  sync_preset_index();
}

int32_t SpinnerWidget::get_value() const {
  if (owns_value_) {
    return internal_value_;
  }
  return value_ptr_ ? *value_ptr_ : 0;
}

void SpinnerWidget::set_value(int32_t value) {
  if (owns_value_) {
    internal_value_ = value;
  } else if (value_ptr_) {
    *value_ptr_ = value;
  }
  sync_preset_index();
  update_display();
}

int32_t* SpinnerWidget::value_storage() {
  return owns_value_ ? &internal_value_ : value_ptr_;
}

void SpinnerWidget::render(lv_obj_t* parent, int32_t y_offset) {
  destroy();

  // Create row container for horizontal layout
  lv_row_ = lv_obj_create(parent);
  lv_obj_remove_style_all(lv_row_);
  lv_obj_set_size(lv_row_, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_pos(lv_row_, 0, y_offset);
  lv_obj_set_style_pad_all(lv_row_, 0, 0);
  lv_obj_set_flex_flow(lv_row_, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(lv_row_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
  lv_obj_clear_flag(lv_row_, LV_OBJ_FLAG_SCROLLABLE);

  // Create prefix label if set
  if (label_ != nullptr) {
    lv_label_prefix_ = lv_label_create(lv_row_);
    lv_label_set_text(lv_label_prefix_, label_);
    if (font_) {
      lv_obj_set_style_text_font(lv_label_prefix_, font_, 0);
    }
    lv_obj_set_style_pad_left(lv_label_prefix_, 2, 0);
  }

  // Create value label
  lv_label_value_ = lv_label_create(lv_row_);
  if (font_) {
    lv_obj_set_style_text_font(lv_label_value_, font_, 0);
  }
  // Add left padding if no prefix
  if (label_ == nullptr) {
    lv_obj_set_style_pad_left(lv_label_value_, 2, 0);
  }

  update_display();
  update_appearance();
}

void SpinnerWidget::destroy() {
  // Only need to delete the row - children are deleted automatically
  if (lv_row_ != nullptr) {
    if (lvgl_port_lock(1000)) {
      lv_obj_del(lv_row_);
      lvgl_port_unlock();
    }
    lv_row_ = nullptr;
    lv_label_prefix_ = nullptr;
    lv_label_value_ = nullptr;
  }
}

void SpinnerWidget::set_focused(bool focused) {
  if (focused_ != focused) {
    focused_ = focused;
    // Exit edit mode when losing focus
    if (!focused && editing_) {
      editing_ = false;
      if (on_commit_) {
        on_commit_(*this, get_value());
      }
    }
    update_appearance();
  }
}

InputResult SpinnerWidget::handle_input(bool short_press) {
  if (short_press) {
    if (editing_) {
      // Cycle value while editing
      cycle_value();
      return InputResult::kConsumed;
    } else {
      // Move to next item
      return InputResult::kFocusNext;
    }
  } else {
    // Long press
    if (editing_) {
      // Exit edit mode, commit value
      editing_ = false;
      update_appearance();
      if (on_commit_) {
        on_commit_(*this, get_value());
      }
      return InputResult::kDeselected;
    } else {
      // Enter edit mode
      editing_ = true;
      update_appearance();
      return InputResult::kSelected;
    }
  }
}

void SpinnerWidget::cycle_value() {
  int32_t* storage = value_storage();
  if (storage == nullptr) {
    return;
  }

  if (presets_ != nullptr && preset_count_ > 0) {
    // Cycle through presets
    preset_index_ = (preset_index_ + 1) % preset_count_;
    *storage = presets_[preset_index_];
  } else {
    // Cycle through range
    int32_t new_value = *storage + step_;
    if (new_value > max_value_) {
      // If we can't reach max exactly with step, allow one more stop at max
      if (*storage < max_value_) {
        new_value = max_value_;
      } else {
        new_value = min_value_;
      }
    } else if (new_value < min_value_) {
      new_value = max_value_;
    }
    *storage = new_value;
  }

  update_display();

  if (on_changed_) {
    on_changed_(*this, get_value());
  }
}

void SpinnerWidget::update_display() {
  if (lv_label_value_ == nullptr) {
    return;
  }

  char buffer[64];
  if (format_) {
    format_(get_value(), buffer, sizeof(buffer));
  } else {
    snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(get_value()));
  }

  lv_label_set_text(lv_label_value_, buffer);
}

void SpinnerWidget::update_appearance() {
  if (lv_row_ == nullptr || lv_label_value_ == nullptr) {
    return;
  }

  if (editing_) {
    // Editing: only value is highlighted
    // Row background transparent
    lv_obj_set_style_bg_opa(lv_row_, LV_OPA_TRANSP, 0);
    // Prefix normal colors
    if (lv_label_prefix_ != nullptr) {
      lv_obj_set_style_text_color(lv_label_prefix_, lv_color_white(), 0);
    }
    // Value highlighted (inverse)
    lv_obj_set_style_bg_color(lv_label_value_, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(lv_label_value_, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(lv_label_value_, lv_color_black(), 0);
  } else if (focused_) {
    // Focused but not editing: entire row highlighted
    lv_obj_set_style_bg_color(lv_row_, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(lv_row_, LV_OPA_COVER, 0);
    // All text black
    if (lv_label_prefix_ != nullptr) {
      lv_obj_set_style_text_color(lv_label_prefix_, lv_color_black(), 0);
    }
    lv_obj_set_style_bg_opa(lv_label_value_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_text_color(lv_label_value_, lv_color_black(), 0);
  } else {
    // Normal: transparent background, white text
    lv_obj_set_style_bg_opa(lv_row_, LV_OPA_TRANSP, 0);
    if (lv_label_prefix_ != nullptr) {
      lv_obj_set_style_text_color(lv_label_prefix_, lv_color_white(), 0);
    }
    lv_obj_set_style_bg_opa(lv_label_value_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_text_color(lv_label_value_, lv_color_white(), 0);
  }
}

void SpinnerWidget::sync_preset_index() {
  if (presets_ == nullptr || preset_count_ == 0) {
    return;
  }

  int32_t current = get_value();
  for (size_t i = 0; i < preset_count_; i++) {
    if (presets_[i] == current) {
      preset_index_ = i;
      return;
    }
  }
  // If not found, default to first preset
  preset_index_ = 0;
}

}  // namespace ui
}  // namespace ezmodes
