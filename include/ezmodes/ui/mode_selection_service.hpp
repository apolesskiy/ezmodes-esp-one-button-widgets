#pragma once

/**
 * @file mode_selection_service.hpp
 * @brief Service providing mode selection menu functionality.
 *
 * Creates pre-built menus from the ezmodes framework's mode registry.
 * Applications can customize the menu by adding extra items via a callback.
 */

#include "ezmodes/modal_framework.hpp"
#include "ezmodes/ui/menu.hpp"
#include "ezmodes/ui/menu_controller.hpp"

#include <functional>
#include <memory>

namespace ezmodes {

/**
 * @brief Callback to add custom menu items after the mode list.
 *
 * Called by build_menu() to let the application inject extra items
 * (e.g. "Settings", "Power Off") without coupling this service to
 * application-specific types.
 *
 * @param menu The menu being built (add children to it)
 * @param controller The MenuController managing the menu
 */
using MenuCustomizer = std::function<void(ui::Menu& menu,
                                          ui::MenuController* controller)>;

/**
 * @brief Service providing mode selection menu functionality.
 *
 * Owned by the application. Provides a factory method to create a
 * pre-built menu for switching modes via the ezmodes framework.
 */
class ModeSelectionService {
 public:
  /**
   * @brief Construct the service.
   * @param framework Pointer to the modal framework (must outlive service)
   * @param config Menu configuration (font, etc.) used when building menus
   */
  ModeSelectionService(ModalFramework* framework,
                       const ui::MenuConfig& config = ui::MenuConfig{});

  ~ModeSelectionService() = default;

  // Non-copyable
  ModeSelectionService(const ModeSelectionService&) = delete;
  ModeSelectionService& operator=(const ModeSelectionService&) = delete;

  /**
   * @brief Set a callback to add custom items to built menus.
   *
   * The customizer is called after the mode list items and before
   * the "Back" item. Use it to inject application-specific actions
   * like "Settings" or "Power Off".
   *
   * @param customizer Callback to add extra menu items
   */
  void set_customizer(MenuCustomizer customizer);

  /**
   * @brief Create a mode selection menu widget tree.
   *
   * The returned menu contains ActionWidgets for each selectable mode
   * (excluding the currently active mode), any custom items injected
   * by the customizer, and a "Back" item. When a mode is selected,
   * it requests a framework transition.
   *
   * @param controller MenuController that will manage this menu (for Back)
   * @return Unique pointer to the menu (caller takes ownership)
   */
  std::unique_ptr<ui::Menu> build_menu(ui::MenuController* controller);

 private:
  ModalFramework* framework_;
  ui::MenuConfig config_;
  MenuCustomizer customizer_;
};

}  // namespace ezmodes
