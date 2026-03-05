/**
 * @file lvgl.h
 * @brief Stub for LVGL for host testing.
 *
 * Provides minimal type definitions and function stubs to allow
 * widget and menu system code to compile for host-based unit testing.
 */

#pragma once

#include <cstdint>
#include <cstdio>

// Basic LVGL types
typedef struct _lv_obj_t {
  int id;
  struct _lv_obj_t* parent;
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
  char text[128];
  bool deleted;
} lv_obj_t;

typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} lv_color_t;

typedef struct {
  int32_t line_height;
} lv_font_t;

// Global font stubs
static lv_font_t lv_font_montserrat_10 = {10};

// Opacity
#define LV_OPA_TRANSP 0
#define LV_OPA_COVER 255

// Size constants
#define LV_SIZE_CONTENT 0xFFFF

// Flex flow
#define LV_FLEX_FLOW_COLUMN 1
#define LV_FLEX_FLOW_ROW 0

// Flex align
#define LV_FLEX_ALIGN_START 0
#define LV_FLEX_ALIGN_CENTER 1

// Alignment
#define LV_ALIGN_LEFT_MID 0
#define LV_ALIGN_CENTER 1
#define LV_ALIGN_RIGHT_MID 2
#define LV_ALIGN_TOP_LEFT 3

// Animation
#define LV_ANIM_OFF 0

// Label long mode
#define LV_LABEL_LONG_WRAP 0
#define LV_LABEL_LONG_DOT 1
#define LV_LABEL_LONG_SCROLL 2
#define LV_LABEL_LONG_SCROLL_CIRCULAR 3
#define LV_LABEL_LONG_CLIP 4

// Scrollbar mode
#define LV_SCROLLBAR_MODE_OFF 0

// Object flags
#define LV_OBJ_FLAG_SCROLLABLE (1 << 0)

// Mock object tracking
namespace lv_mock {
  void reset();
  lv_obj_t* get_last_created();
}

// Percentage helper
inline int32_t lv_pct(int32_t v) { return v | 0x8000; }

// Color helpers
inline lv_color_t lv_color_white() { return {255, 255, 255}; }
inline lv_color_t lv_color_black() { return {0, 0, 0}; }

// Object creation — declarations only, defined in test file
lv_obj_t* lv_obj_create(lv_obj_t* parent);
lv_obj_t* lv_label_create(lv_obj_t* parent);

// Object deletion
inline void lv_obj_del(lv_obj_t* obj) {
  if (obj != nullptr) {
    obj->deleted = true;
  }
}

// Position and size
inline void lv_obj_set_pos(lv_obj_t* obj, int32_t x, int32_t y) {
  if (obj) { obj->x = x; obj->y = y; }
}
inline void lv_obj_set_size(lv_obj_t* obj, int32_t w, int32_t h) {
  if (obj) { obj->width = w; obj->height = h; }
}
inline void lv_obj_set_width(lv_obj_t* obj, int32_t w) {
  if (obj) { obj->width = w; }
}
inline void lv_obj_set_height(lv_obj_t* obj, int32_t h) {
  if (obj) { obj->height = h; }
}

// Label
inline void lv_label_set_text(lv_obj_t* obj, const char* text) {
  if (obj && text) {
    snprintf(obj->text, sizeof(obj->text), "%s", text);
  }
}
inline void lv_label_set_long_mode(lv_obj_t*, int32_t) {}

// Alignment
inline void lv_obj_align(lv_obj_t*, int32_t, int32_t, int32_t) {}
inline void lv_obj_center(lv_obj_t*) {}
inline void lv_obj_align_to(lv_obj_t*, lv_obj_t*, int32_t, int32_t, int32_t) {}

// Styling (no-ops)
inline void lv_obj_set_style_pad_all(lv_obj_t*, int32_t, int32_t) {}
inline void lv_obj_set_style_pad_row(lv_obj_t*, int32_t, int32_t) {}
inline void lv_obj_set_style_pad_left(lv_obj_t*, int32_t, int32_t) {}
inline void lv_obj_set_style_pad_top(lv_obj_t*, int32_t, int32_t) {}
inline void lv_obj_set_style_pad_column(lv_obj_t*, int32_t, int32_t) {}
inline void lv_obj_set_style_border_width(lv_obj_t*, int32_t, int32_t) {}
inline void lv_obj_set_style_bg_opa(lv_obj_t*, uint8_t, int32_t) {}
inline void lv_obj_set_style_bg_color(lv_obj_t*, lv_color_t, int32_t) {}
inline void lv_obj_set_style_text_color(lv_obj_t*, lv_color_t, int32_t) {}
inline void lv_obj_set_style_text_font(lv_obj_t*, const lv_font_t*, int32_t) {}
inline void lv_obj_set_style_text_align(lv_obj_t*, int32_t, int32_t) {}
inline void lv_obj_set_flex_flow(lv_obj_t*, int32_t) {}
inline void lv_obj_set_flex_align(lv_obj_t*, int32_t, int32_t, int32_t) {}
inline void lv_obj_set_flex_grow(lv_obj_t*, int32_t) {}
inline void lv_obj_remove_style_all(lv_obj_t*) {}

// Scrolling
inline void lv_obj_scroll_to_y(lv_obj_t*, int32_t, int32_t) {}
inline int32_t lv_obj_get_scroll_y(lv_obj_t*) { return 0; }

// Size queries
inline int32_t lv_obj_get_height(lv_obj_t* obj) { return obj ? obj->height : 0; }
inline int32_t lv_obj_get_content_height(lv_obj_t* obj) { return obj ? obj->height : 0; }
inline int32_t lv_obj_get_y(lv_obj_t* obj) { return obj ? obj->y : 0; }

// Object flags
inline void lv_obj_add_flag(lv_obj_t*, uint32_t) {}
inline void lv_obj_clear_flag(lv_obj_t*, uint32_t) {}

// Scrollbar
inline void lv_obj_set_scrollbar_mode(lv_obj_t*, int32_t) {}

// Text align
#define LV_TEXT_ALIGN_LEFT 0
#define LV_TEXT_ALIGN_RIGHT 1
