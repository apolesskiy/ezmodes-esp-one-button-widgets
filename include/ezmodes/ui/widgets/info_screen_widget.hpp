#pragma once

/**
 * @file info_screen_widget.hpp
 * @brief Read-only multi-line text display widget.
 *
 * Displays a fixed set of text lines using a caller-supplied font.
 * Designed for "about" / "info" screens. Long-press invokes a
 * callback (typically used to pop the menu and go back).
 *
 * Input behavior:
 * - Short press: kConsumed (no navigation)
 * - Long press: Invokes on_back callback, returns kSelected
 */

#include "ezmodes/ui/widget.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>

namespace ezmodes {
namespace ui {

class InfoScreenWidget;

/**
 * @brief Callback invoked on long-press (back action).
 */
using InfoScreenBackCallback = std::function<void(InfoScreenWidget& widget)>;

/**
 * @brief Read-only multi-line text display widget.
 *
 * Renders up to kMaxLines lines of text using a dedicated display
 * font (independent of the Menu's font_large).
 */
class InfoScreenWidget : public Widget {
 public:
  static constexpr size_t kMaxLines = 8;

  /**
   * @brief Construct an info screen widget.
   * @param display_font Font used to render the text lines
   */
  explicit InfoScreenWidget(const lv_font_t* display_font);

  ~InfoScreenWidget() override;

  InfoScreenWidget(const InfoScreenWidget&) = delete;
  InfoScreenWidget& operator=(const InfoScreenWidget&) = delete;

  // --- Content ---

  /**
   * @brief Set the text for a specific line.
   * @param index Line index (0-based, must be < kMaxLines)
   * @param text  Text to display (copied internally)
   */
  void set_line(size_t index, const char* text);

  // --- Events ---

  /**
   * @brief Set callback for back action (long-press).
   */
  void set_on_back(InfoScreenBackCallback callback);

  // --- Widget Interface ---

  void render(lv_obj_t* parent, int32_t y_offset) override;
  void destroy() override;
  void set_focused(bool focused) override;
  bool is_focused() const override { return focused_; }
  InputResult handle_input(bool short_press) override;
  int32_t get_height() const override;

 private:
  const lv_font_t* display_font_;
  char lines_[kMaxLines][48];
  size_t line_count_ = 0;
  lv_obj_t* lv_labels_[kMaxLines] = {};
  bool focused_ = true;
  InfoScreenBackCallback on_back_;
};

}  // namespace ui
}  // namespace ezmodes
