/**
 * @file main.cpp
 * @brief Example: Building a settings menu with one-button widgets.
 *
 * Demonstrates creating a menu with action, spinner, toggle, and submenu
 * widgets using the ezmodes one-button widget library.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ezmodes/ui/menu.hpp"
#include "ezmodes/ui/menu_controller.hpp"
#include "ezmodes/ui/widgets/action_widget.hpp"
#include "ezmodes/ui/widgets/spinner_widget.hpp"
#include "ezmodes/ui/widgets/submenu_widget.hpp"
#include "ezmodes/ui/widgets/toggle_widget.hpp"
#include "ezmodes/ui/widgets/info_screen_widget.hpp"

#include "esp_log.h"

static const char* TAG = "menu_example";

/**
 * @brief Build a demo settings menu.
 * @param screen LVGL screen to render on
 * @param font Font to use for menu items
 */
void build_demo_menu(lv_obj_t* screen, const lv_font_t* font) {
  using namespace ezmodes::ui;

  // Create controller for the screen
  static MenuController controller(screen);

  // Create a root menu
  MenuConfig config{font};
  static auto root = std::make_unique<Menu>("Demo", config);

  // Action widget: a simple button
  auto save = std::make_unique<ActionWidget>("Save");
  save->set_on_selected([](ActionWidget& w) {
    (void)w;
    ESP_LOGI(TAG, "Save selected!");
  });
  root->add_child(std::move(save));

  // Spinner widget: brightness control (0-100, step 10)
  auto brightness = std::make_unique<SpinnerWidget>(50, 0, 100, 10);
  brightness->set_label("Bright: ");
  brightness->set_format([](int32_t v, char* buf, size_t sz) {
    snprintf(buf, sz, "%d%%", static_cast<int>(v));
  });
  brightness->set_on_commit([](SpinnerWidget& w, int32_t val) {
    ESP_LOGI(TAG, "Brightness committed: %d", static_cast<int>(val));
  });
  root->add_child(std::move(brightness));

  // Toggle widget: sound on/off
  auto sound = std::make_unique<ToggleWidget>("Sound", true);
  sound->set_on_changed([](ToggleWidget& w, bool state) {
    ESP_LOGI(TAG, "Sound: %s", state ? "On" : "Off");
  });
  root->add_child(std::move(sound));

  // Submenu widget: advanced settings
  auto advanced = std::make_unique<Menu>("Advanced", config);
  auto reset = std::make_unique<ActionWidget>("Reset Defaults");
  reset->set_on_selected([](ActionWidget& w) {
    (void)w;
    ESP_LOGI(TAG, "Defaults reset!");
  });
  advanced->add_child(std::move(reset));

  auto submenu = std::make_unique<SubmenuWidget>("Advanced >",
                                                  std::move(advanced));
  root->add_child(std::move(submenu));

  // Info screen: read-only display with back on long-press
  auto info_menu = std::make_unique<Menu>(nullptr, config);
  auto info_widget = std::make_unique<InfoScreenWidget>(font);
  info_widget->set_line(0, "SN:AABBCCDDEEFF");
  info_widget->set_line(1, "Fw:example_app");
  info_widget->set_line(2, "V:1.0.0");
  info_widget->set_line(3, "#:abcde");
  info_widget->set_on_back([&controller](InfoScreenWidget& w) {
    (void)w;
    controller.pop();
  });
  info_menu->add_child(std::move(info_widget));

  auto info_sub = std::make_unique<SubmenuWidget>("Info >",
                                                   std::move(info_menu));
  root->add_child(std::move(info_sub));

  // Push root menu
  controller.push(root.get());

  // In your button handler, call:
  // controller.handle_input(short_press);
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "One-button widgets example starting");

  // NOTE: LVGL display must be initialized before calling build_demo_menu().
  // See your board's display driver documentation for initialization.

  // Once display is ready, call:
  // build_demo_menu(lv_scr_act(), &lv_font_montserrat_14);

  // Keep main task alive
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
