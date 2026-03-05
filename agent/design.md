# ezmodes-esp-one-button-widgets — Architecture & Design

## Overview

Single-button widget and menu system for ESP32 with LVGL. Provides a complete
UI toolkit designed for devices with a single button (short press / long press).

## Architecture

### Widget Interface

All interactive elements implement the `Widget` base class:

- `render(parent, y_offset)` — Create/update LVGL objects on the given parent.
- `destroy()` — Remove LVGL objects.
- `set_focused(focused)` / `is_focused()` — Visual focus state.
- `handle_input(short_press)` — Process button input, return `InputResult`.
- `is_focusable()` — Whether the widget can receive focus.
- `get_height()` — Widget height for layout.

### InputResult Flow

The `InputResult` enum drives the navigation model:

| Value | Meaning | Controller Action |
|-------|---------|-------------------|
| `kConsumed` | Input handled internally | No action |
| `kFocusNext` | Move focus forward | `focus_next()` |
| `kFocusPrev` | Move focus backward | `focus_prev()` |
| `kSelected` | Widget activated | Push submenu or notify |
| `kDeselected` | Widget deactivated | No action |
| `kNotHandled` | Input not consumed | Propagate to parent |

### Menu Container

`Menu` is a vertical list of child `Widget` instances:

- Owns children via `unique_ptr<Widget>`.
- Tracks focusable vs. non-focusable children.
- Propagates font configuration from `MenuConfig` to children on render.
- Not focusable itself (container only).
- Optional title string for display.

### MenuController

Stack-based navigation controller:

- Maintains a stack of `Menu*` pointers with saved focus indices.
- `push()` adds a menu; `pop()` returns to previous with restored focus.
- `handle_input()` routes to focused widget, interprets `InputResult`.
- Automatically pushes submenus when `SubmenuWidget` returns `kSelected`.
- `push_menu()` takes owned `unique_ptr<Menu>` for dynamically built menus.

### Widget Types

| Widget | Short Press | Long Press | Description |
|--------|------------|------------|-------------|
| `ActionWidget` | kFocusNext | Fires callback, kSelected | Simple selectable action |
| `SpinnerWidget` | kFocusNext (normal) / cycle (editing) | Enter/exit edit mode | Cyclic value editor |
| `ToggleWidget` | kFocusNext | Toggle value, kConsumed | Boolean toggle |
| `SubmenuWidget` | kFocusNext | kSelected (controller pushes) | Submenu navigation |

### SpinnerWidget Two-Phase Editing

1. **Normal**: Short press = kFocusNext, long press = enter edit mode (kSelected).
2. **Editing**: Short press = cycle value, long press = commit (kDeselected).
   - Supports range mode (min/max/step with wrapping) and presets mode.
   - `on_changed` fires during editing; `on_commit` fires on commit.
   - Focus loss auto-commits (safety feature).

### InlineMenu

Lightweight alternative to Menu/MenuController for simple use cases:

- Flat list of `MenuItem` structs with label, optional value text.
- Supports cycle-edit and select callbacks.
- Single-level, no nested navigation.
- Independent system from Menu/MenuController.

### ModeSelectionService

Builds a mode-switching menu from the `ModalFramework` mode registry:

- Factory method `build_menu()` returns `unique_ptr<Menu>`.
- Lists all selectable modes (excluding currently active).
- Supports `MenuCustomizer` callback for injecting custom items.
- Adds "Back" action that pops the menu.

## Key Design Decisions

### Composition Over Inheritance
Menu containers own widgets via `unique_ptr`. Widgets are self-contained UI
components that know how to render, handle focus, and process input.

### InputResult-Driven Navigation
Rather than widgets directly manipulating the controller, they return
`InputResult` values that the controller interprets. This decouples
widgets from navigation logic.

### Two Systems
`Menu`/`MenuController` (structured, nested) and `InlineMenu` (lightweight,
flat) serve different complexity needs. They share the same LVGL rendering
approach but have independent APIs.

### Focus Wrapping
Focus indices wrap around in both directions (next wraps to 0, prev wraps
to last). This enables single-button navigation through all items.

### Stack-Based Submenu Navigation
The controller uses a stack with saved focus indices. Popping restores
the exact focus position from before entering the submenu.

## API Surface

| Component | Key Members |
|-----------|-------------|
| `Widget` | 7 virtual methods (2 with defaults) |
| `InputResult` | 6 enum values |
| `Menu` | 8 public methods, owns children |
| `MenuController` | 12 public methods, stack-based |
| `ActionWidget` | label, callback, set_label |
| `SpinnerWidget` | value, range/presets, edit mode, 3 callbacks |
| `ToggleWidget` | bool value, callback, custom state text |
| `SubmenuWidget` | label, owned submenu |
| `InlineMenu` | MenuItem list, 3 callback types |
| `ModeSelectionService` | build_menu factory, customizer callback |
| `MenuConfig` | font_large, font_small, item_height() |

## Dependencies

- ESP-IDF ≥ 5.5.0 (`log`, `esp_lvgl_port`, `freertos`)
- `ezmodes_esp_base` ≥ 0.1.0 (for ModeSelectionService only)
- LVGL (via `esp_lvgl_port`)
