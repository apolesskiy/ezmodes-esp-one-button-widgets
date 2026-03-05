/**
 * @file mode_selection_service.cpp
 * @brief Implementation of ModeSelectionService.
 */

#include "ezmodes/ui/mode_selection_service.hpp"

#include "ezmodes/ui/menu_controller.hpp"
#include "ezmodes/ui/widgets/action_widget.hpp"

#include "esp_log.h"

#include <cstring>

static const char* TAG = "mode_selection_svc";

namespace ezmodes {

ModeSelectionService::ModeSelectionService(ModalFramework* framework,
                                           const ui::MenuConfig& config)
    : framework_(framework), config_(config) {}

void ModeSelectionService::set_customizer(MenuCustomizer customizer) {
  customizer_ = std::move(customizer);
}

std::unique_ptr<ui::Menu> ModeSelectionService::build_menu(
    ui::MenuController* controller) {
  auto menu = std::make_unique<ui::Menu>("System", config_);

  // Get list of selectable modes from framework
  std::vector<ModeInterface*> selectable_modes =
      framework_->get_selectable_modes();

  // Get the currently active mode to exclude it from the list
  ModeInterface* active_mode = framework_->get_active_mode();
  const char* active_id = active_mode ? active_mode->get_id() : nullptr;

  ESP_LOGI(TAG, "Building menu with %zu selectable modes",
           selectable_modes.size());

  // Add an ActionWidget for each selectable mode (except current)
  for (ModeInterface* mode : selectable_modes) {
    const char* mode_id = mode->get_id();
    const char* display_name = mode->get_display_name();

    // Skip the currently active mode
    if (active_id != nullptr && strcmp(mode_id, active_id) == 0) {
      continue;
    }

    auto action = std::make_unique<ui::ActionWidget>(display_name);

    // Capture framework and mode_id for the callback
    ModalFramework* fw = framework_;
    action->set_on_selected([fw, mode_id](ui::ActionWidget& widget) {
      (void)widget;
      ESP_LOGI(TAG, "Switching to mode: %s", mode_id);
      fw->request_transition(mode_id);
    });

    menu->add_child(std::move(action));
  }

  // Allow the application to inject custom items (e.g., Settings, Power Off)
  if (customizer_) {
    customizer_(*menu, controller);
  }

  // Add Back option
  auto back = std::make_unique<ui::ActionWidget>("Back");
  back->set_on_selected([controller](ui::ActionWidget& widget) {
    (void)widget;
    if (controller != nullptr) {
      controller->pop();
    }
  });
  menu->add_child(std::move(back));

  return menu;
}

}  // namespace ezmodes
