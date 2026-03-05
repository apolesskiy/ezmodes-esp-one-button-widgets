# ezmodes_esp_one_button_widgets

Widget and menu system for single-button UIs on ESP32 with LVGL.
Designed for use with [`ezmodes-esp-base`](https://github.com/apolesskiy/ezmodes-esp-base) but can be used independently
with any LVGL application.

## Features

- **Menu system**: Vertical menu containers with scrolling, nested submenu
  navigation, and a controller that manages focus and input routing.
- **Widget library**: Pre-built widgets for common UI patterns:
  - `ActionWidget` — selectable button/menu item
  - `SpinnerWidget` — cyclic value editor with presets
  - `ToggleWidget` — boolean on/off toggle
  - `SubmenuWidget` — opens a nested submenu
- **InlineMenu** — lightweight scrollable menu with inline value editing
- **ModeSelectionService** — factory that builds a mode-switching menu
  from the `ezmodes` framework's mode registry
- **Single-button input model**: Short press cycles focus, long press
  selects/edits

## Installation

### ESP Component Registry

```yaml
# idf_component.yml
dependencies:
  apolesskiy/ezmodes-esp-one-button-widgets:
    git: https://github.com/apolesskiy/ezmodes-esp-one-button-widgets.git
```

### Manual

Clone into your project's `components/` directory:

```bash
cd components
git clone https://github.com/apolesskiy/ezmodes-esp-one-button-widgets.git ezmodes-esp-one-button-widgets
```

## Quick Start

### Build a Menu

```cpp
#include "ezmodes/ui/menu.hpp"
#include "ezmodes/ui/menu_controller.hpp"
#include "ezmodes/ui/widgets/action_widget.hpp"
#include "ezmodes/ui/widgets/spinner_widget.hpp"
#include "ezmodes/ui/widgets/toggle_widget.hpp"

using namespace ezmodes::ui;

// Create a menu with items
MenuConfig config{&my_font};
auto menu = std::make_unique<Menu>("Settings", config);

// Add an action button
auto action = std::make_unique<ActionWidget>("Save");
action->set_on_selected([](ActionWidget&) { /* save logic */ });
menu->add_child(std::move(action));

// Add a spinner
auto brightness = std::make_unique<SpinnerWidget>(50, 0, 100, 10);
brightness->set_label("Bright: ");
brightness->set_format([](int32_t v, char* buf, size_t sz) {
  snprintf(buf, sz, "%d%%", (int)v);
});
menu->add_child(std::move(brightness));

// Add a toggle
auto sound = std::make_unique<ToggleWidget>("Sound", true);
menu->add_child(std::move(sound));
```

### Use MenuController

```cpp
// Create controller for a screen
MenuController controller(screen);
controller.push(menu.get());

// Route button events
void on_button(bool short_press) {
  controller.handle_input(short_press);
}
```

### ModeSelectionService

```cpp
#include "ezmodes/ui/mode_selection_service.hpp"

// Create service (pass framework pointer)
ezmodes::ModeSelectionService mode_svc(&framework);

// Build a mode selection menu
auto mode_menu = mode_svc.build_menu(&controller);
controller.push_menu(std::move(mode_menu));
```

## Input Model

Navigation uses two button actions:

| Action | In navigation | In editing |
|--------|--------------|------------|
| Short press | Focus next widget | Cycle value |
| Long press | Select/enter edit | Commit and exit |

## Widget Types

### ActionWidget

Simple selectable item. Short press advances focus, long press fires callback.

### SpinnerWidget

Editable numeric value with min/max/step or discrete presets. Shows a
label prefix and highlighted value portion. Supports external value
storage via pointer or internal storage.

### ToggleWidget

Boolean toggle. Long press flips state. Displays "On"/"Off" (customizable).

### SubmenuWidget

Menu item that opens a nested menu when selected. Displays with ">" indicator.

### InlineMenu

Lightweight scrollable menu with inline value editing. Simpler alternative
to the Menu/MenuController system for flat menu structures.

## Architecture

```
ezmodes_esp_one_button_widgets/
├── include/ezmodes/ui/
│   ├── widget.hpp          # Base widget interface
│   ├── menu.hpp            # Menu container
│   ├── menu_config.hpp     # Font/layout configuration
│   ├── menu_controller.hpp # Navigation controller
│   ├── inline_menu.hpp     # Lightweight inline menu
│   ├── mode_selection_service.hpp
│   └── widgets/
│       ├── action_widget.hpp
│       ├── spinner_widget.hpp
│       ├── submenu_widget.hpp
│       └── toggle_widget.hpp
└── src/
    ├── menu.cpp
    ├── menu_controller.cpp
    ├── inline_menu.cpp
    ├── mode_selection_service.cpp
    └── widgets/
        ├── action_widget.cpp
        ├── spinner_widget.cpp
        ├── submenu_widget.cpp
        └── toggle_widget.cpp
```

## Requirements

- ESP-IDF v5.5.0 or later
- LVGL (via `esp_lvgl_port`)
- `apolesskiy/ezmodes-esp-base` (for `ModeSelectionService` only)
