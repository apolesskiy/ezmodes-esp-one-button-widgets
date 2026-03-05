/**
 * @file test_widgets.cpp
 * @brief Host-based unit tests for the widget and menu system.
 *
 * Tests Widget, Menu, MenuController, ActionWidget, SpinnerWidget,
 * ToggleWidget, and SubmenuWidget without requiring actual LVGL rendering.
 */

#include "lvgl.h"  // Must come first to define lv_mock functions

#include <vector>

// LVGL mock implementation
namespace {
std::vector<lv_obj_t*> created_objects;
int next_id = 1;
}  // namespace

namespace lv_mock {
void reset() {
  for (auto* obj : created_objects) {
    delete obj;
  }
  created_objects.clear();
  next_id = 1;
}

lv_obj_t* get_last_created() {
  if (created_objects.empty()) return nullptr;
  return created_objects.back();
}
}  // namespace lv_mock

lv_obj_t* lv_obj_create(lv_obj_t* parent) {
  lv_obj_t* obj = new lv_obj_t();
  obj->id = next_id++;
  obj->parent = parent;
  obj->x = 0;
  obj->y = 0;
  obj->width = 0;
  obj->height = 0;
  obj->text[0] = '\0';
  obj->deleted = false;
  created_objects.push_back(obj);
  return obj;
}

lv_obj_t* lv_label_create(lv_obj_t* parent) {
  return lv_obj_create(parent);
}

#include "ezmodes/ui/menu.hpp"
#include "ezmodes/ui/menu_controller.hpp"
#include "ezmodes/ui/widgets/action_widget.hpp"
#include "ezmodes/ui/widgets/submenu_widget.hpp"
#include "ezmodes/ui/widgets/spinner_widget.hpp"
#include "ezmodes/ui/widgets/toggle_widget.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <memory>

using namespace ezmodes::ui;

namespace {

int tests_passed = 0;
int tests_failed = 0;

#define TEST_ASSERT(expr)                                                 \
  do {                                                                    \
    if (!(expr)) {                                                        \
      printf("FAIL: %s:%d: %s\n", __FILE__, __LINE__, #expr);            \
      tests_failed++;                                                     \
      return;                                                             \
    }                                                                     \
  } while (0)

#define RUN_TEST(test_func)                                               \
  do {                                                                    \
    printf("Running %s... ", #test_func);                                 \
    lv_mock::reset();                                                     \
    test_func();                                                          \
    printf("PASS\n");                                                     \
    tests_passed++;                                                       \
  } while (0)

// ============================================================================
// ActionWidget Tests
// ============================================================================

void test_action_widget_creation() {
  ActionWidget widget("Test Label");
  TEST_ASSERT(widget.get_label() != nullptr);
  TEST_ASSERT(strcmp(widget.get_label(), "Test Label") == 0);
  TEST_ASSERT(!widget.is_focused());
  TEST_ASSERT(widget.is_focusable());
}

void test_action_widget_focus() {
  ActionWidget widget("Focus Test");
  TEST_ASSERT(!widget.is_focused());

  widget.set_focused(true);
  TEST_ASSERT(widget.is_focused());

  widget.set_focused(false);
  TEST_ASSERT(!widget.is_focused());
}

void test_action_widget_input_short_press() {
  ActionWidget widget("Input Test");

  // Short press should return FocusNext.
  InputResult result = widget.handle_input(true);
  TEST_ASSERT(result == InputResult::kFocusNext);
}

void test_action_widget_input_long_press() {
  ActionWidget widget("Input Test");

  bool callback_called = false;
  widget.set_on_selected([&callback_called](ActionWidget&) {
    callback_called = true;
  });

  // Long press should call callback and return Selected.
  InputResult result = widget.handle_input(false);
  TEST_ASSERT(result == InputResult::kSelected);
  TEST_ASSERT(callback_called);
}

void test_action_widget_callback_receives_widget_reference() {
  ActionWidget widget("Ref Test");

  ActionWidget* received_widget = nullptr;
  widget.set_on_selected([&received_widget](ActionWidget& w) {
    received_widget = &w;
  });

  widget.handle_input(false);
  TEST_ASSERT(received_widget == &widget);
}

void test_action_widget_set_label() {
  ActionWidget widget("Initial");
  TEST_ASSERT(strcmp(widget.get_label(), "Initial") == 0);

  widget.set_label("Updated");
  TEST_ASSERT(strcmp(widget.get_label(), "Updated") == 0);
}

// ============================================================================
// Menu Container Tests
// ============================================================================

void test_menu_creation() {
  Menu menu("Test Menu");
  TEST_ASSERT(menu.get_title() != nullptr);
  TEST_ASSERT(strcmp(menu.get_title(), "Test Menu") == 0);
  TEST_ASSERT(menu.child_count() == 0);
  TEST_ASSERT(menu.focusable_count() == 0);
}

void test_menu_creation_no_title() {
  Menu menu;
  TEST_ASSERT(menu.get_title() == nullptr);
}

void test_menu_add_children() {
  Menu menu;

  menu.add_child(std::make_unique<ActionWidget>("Item 1"));
  TEST_ASSERT(menu.child_count() == 1);
  TEST_ASSERT(menu.focusable_count() == 1);

  menu.add_child(std::make_unique<ActionWidget>("Item 2"));
  TEST_ASSERT(menu.child_count() == 2);
  TEST_ASSERT(menu.focusable_count() == 2);
}

void test_menu_get_child() {
  Menu menu;

  menu.add_child(std::make_unique<ActionWidget>("Item 1"));
  menu.add_child(std::make_unique<ActionWidget>("Item 2"));

  Widget* first = menu.get_child(0);
  Widget* second = menu.get_child(1);
  Widget* invalid = menu.get_child(5);

  TEST_ASSERT(first != nullptr);
  TEST_ASSERT(second != nullptr);
  TEST_ASSERT(invalid == nullptr);
}

void test_menu_get_focusable_child() {
  Menu menu;

  menu.add_child(std::make_unique<ActionWidget>("Item 1"));
  menu.add_child(std::make_unique<ActionWidget>("Item 2"));

  Widget* first = menu.get_focusable_child(0);
  Widget* second = menu.get_focusable_child(1);
  Widget* invalid = menu.get_focusable_child(5);

  TEST_ASSERT(first != nullptr);
  TEST_ASSERT(second != nullptr);
  TEST_ASSERT(invalid == nullptr);
  TEST_ASSERT(first != second);
}

void test_menu_is_not_focusable() {
  Menu menu;
  TEST_ASSERT(!menu.is_focusable());
}

// ============================================================================
// MenuController Tests
// ============================================================================

void test_controller_creation() {
  lv_obj_t screen;
  MenuController controller(&screen);

  TEST_ASSERT(!controller.is_active());
  TEST_ASSERT(controller.stack_depth() == 0);
  TEST_ASSERT(controller.current_menu() == nullptr);
}

void test_controller_push_menu() {
  lv_obj_t screen;
  Menu menu;
  menu.add_child(std::make_unique<ActionWidget>("Item 1"));

  MenuController controller(&screen);
  controller.push(&menu);

  TEST_ASSERT(controller.is_active());
  TEST_ASSERT(controller.stack_depth() == 1);
  TEST_ASSERT(controller.current_menu() == &menu);
}

void test_controller_push_multiple_menus() {
  lv_obj_t screen;

  Menu menu1;
  menu1.add_child(std::make_unique<ActionWidget>("Menu 1 Item"));

  Menu menu2;
  menu2.add_child(std::make_unique<ActionWidget>("Menu 2 Item"));

  MenuController controller(&screen);

  controller.push(&menu1);
  TEST_ASSERT(controller.current_menu() == &menu1);
  TEST_ASSERT(controller.stack_depth() == 1);

  controller.push(&menu2);
  TEST_ASSERT(controller.current_menu() == &menu2);
  TEST_ASSERT(controller.stack_depth() == 2);
}

void test_controller_pop_menu() {
  lv_obj_t screen;

  Menu menu1;
  menu1.add_child(std::make_unique<ActionWidget>("Menu 1 Item"));

  Menu menu2;
  menu2.add_child(std::make_unique<ActionWidget>("Menu 2 Item"));

  MenuController controller(&screen);

  controller.push(&menu1);
  controller.push(&menu2);
  TEST_ASSERT(controller.stack_depth() == 2);

  bool popped = controller.pop();
  TEST_ASSERT(popped);
  TEST_ASSERT(controller.stack_depth() == 1);
  TEST_ASSERT(controller.current_menu() == &menu1);
}

void test_controller_cannot_pop_last_menu() {
  lv_obj_t screen;

  Menu menu;
  menu.add_child(std::make_unique<ActionWidget>("Item"));

  MenuController controller(&screen);

  controller.push(&menu);
  TEST_ASSERT(controller.stack_depth() == 1);

  bool popped = controller.pop();
  TEST_ASSERT(!popped);  // Can't pop the root menu.
  TEST_ASSERT(controller.stack_depth() == 1);
}

void test_controller_clear() {
  lv_obj_t screen;

  Menu menu1;
  menu1.add_child(std::make_unique<ActionWidget>("Item 1"));

  Menu menu2;
  menu2.add_child(std::make_unique<ActionWidget>("Item 2"));

  MenuController controller(&screen);

  controller.push(&menu1);
  controller.push(&menu2);
  TEST_ASSERT(controller.stack_depth() == 2);

  controller.clear();
  TEST_ASSERT(controller.stack_depth() == 0);
  TEST_ASSERT(!controller.is_active());
}

void test_controller_focus_starts_at_zero() {
  lv_obj_t screen;

  Menu menu;
  menu.add_child(std::make_unique<ActionWidget>("Item 1"));
  menu.add_child(std::make_unique<ActionWidget>("Item 2"));

  MenuController controller(&screen);

  controller.push(&menu);
  TEST_ASSERT(controller.get_focus_index() == 0);
}

void test_controller_focus_next() {
  lv_obj_t screen;

  Menu menu;
  menu.add_child(std::make_unique<ActionWidget>("Item 1"));
  menu.add_child(std::make_unique<ActionWidget>("Item 2"));
  menu.add_child(std::make_unique<ActionWidget>("Item 3"));

  MenuController controller(&screen);

  controller.push(&menu);
  TEST_ASSERT(controller.get_focus_index() == 0);

  controller.focus_next();
  TEST_ASSERT(controller.get_focus_index() == 1);

  controller.focus_next();
  TEST_ASSERT(controller.get_focus_index() == 2);

  // Should wrap around.
  controller.focus_next();
  TEST_ASSERT(controller.get_focus_index() == 0);
}

void test_controller_focus_prev() {
  lv_obj_t screen;

  Menu menu;
  menu.add_child(std::make_unique<ActionWidget>("Item 1"));
  menu.add_child(std::make_unique<ActionWidget>("Item 2"));
  menu.add_child(std::make_unique<ActionWidget>("Item 3"));

  MenuController controller(&screen);

  controller.push(&menu);
  TEST_ASSERT(controller.get_focus_index() == 0);

  // Should wrap to last.
  controller.focus_prev();
  TEST_ASSERT(controller.get_focus_index() == 2);

  controller.focus_prev();
  TEST_ASSERT(controller.get_focus_index() == 1);
}

void test_controller_set_focus() {
  lv_obj_t screen;

  Menu menu;
  menu.add_child(std::make_unique<ActionWidget>("Item 1"));
  menu.add_child(std::make_unique<ActionWidget>("Item 2"));
  menu.add_child(std::make_unique<ActionWidget>("Item 3"));

  MenuController controller(&screen);

  controller.push(&menu);

  controller.set_focus(2);
  TEST_ASSERT(controller.get_focus_index() == 2);

  controller.set_focus(0);
  TEST_ASSERT(controller.get_focus_index() == 0);

  // Out of range should be ignored.
  controller.set_focus(10);
  TEST_ASSERT(controller.get_focus_index() == 0);
}

void test_controller_get_focused_widget() {
  lv_obj_t screen;

  Menu menu;
  auto action1 = std::make_unique<ActionWidget>("Item 1");
  auto action2 = std::make_unique<ActionWidget>("Item 2");

  Widget* raw1 = action1.get();
  Widget* raw2 = action2.get();

  menu.add_child(std::move(action1));
  menu.add_child(std::move(action2));

  MenuController controller(&screen);

  controller.push(&menu);

  TEST_ASSERT(controller.get_focused_widget() == raw1);

  controller.focus_next();
  TEST_ASSERT(controller.get_focused_widget() == raw2);
}

void test_controller_handle_input_routes_to_widget() {
  lv_obj_t screen;

  Menu menu;
  auto action = std::make_unique<ActionWidget>("Test");

  bool callback_called = false;
  action->set_on_selected([&callback_called](ActionWidget&) {
    callback_called = true;
  });

  menu.add_child(std::move(action));

  MenuController controller(&screen);
  controller.push(&menu);

  // Long press should invoke widget's callback.
  bool consumed = controller.handle_input(false);
  TEST_ASSERT(consumed);
  TEST_ASSERT(callback_called);
}

void test_controller_short_press_moves_focus() {
  lv_obj_t screen;

  Menu menu;
  menu.add_child(std::make_unique<ActionWidget>("Item 1"));
  menu.add_child(std::make_unique<ActionWidget>("Item 2"));

  MenuController controller(&screen);

  controller.push(&menu);
  TEST_ASSERT(controller.get_focus_index() == 0);

  // Short press should move to next.
  controller.handle_input(true);
  TEST_ASSERT(controller.get_focus_index() == 1);
}

void test_controller_focus_restored_on_pop() {
  lv_obj_t screen;

  Menu menu1;
  menu1.add_child(std::make_unique<ActionWidget>("M1 Item 1"));
  menu1.add_child(std::make_unique<ActionWidget>("M1 Item 2"));
  menu1.add_child(std::make_unique<ActionWidget>("M1 Item 3"));

  Menu menu2;
  menu2.add_child(std::make_unique<ActionWidget>("M2 Item 1"));

  MenuController controller(&screen);

  controller.push(&menu1);
  controller.set_focus(2);  // Focus third item.
  TEST_ASSERT(controller.get_focus_index() == 2);

  controller.push(&menu2);
  TEST_ASSERT(controller.get_focus_index() == 0);  // Reset to 0 for new menu.

  controller.pop();
  TEST_ASSERT(controller.get_focus_index() == 2);  // Restored.
}

// ============================================================================
// SubmenuWidget Tests
// ============================================================================

void test_submenu_widget_creation() {
  auto submenu = std::make_unique<Menu>("Submenu");
  SubmenuWidget widget("Settings", std::move(submenu));

  TEST_ASSERT(widget.get_label() != nullptr);
  TEST_ASSERT(strcmp(widget.get_label(), "Settings") == 0);
  TEST_ASSERT(widget.get_submenu() != nullptr);
  TEST_ASSERT(widget.is_focusable());
}

void test_submenu_widget_input() {
  auto submenu = std::make_unique<Menu>();
  SubmenuWidget widget("Settings", std::move(submenu));

  // Short press moves to next.
  InputResult short_result = widget.handle_input(true);
  TEST_ASSERT(short_result == InputResult::kFocusNext);

  // Long press returns selected (controller handles navigation).
  InputResult long_result = widget.handle_input(false);
  TEST_ASSERT(long_result == InputResult::kSelected);
}

void test_controller_submenu_navigation() {
  lv_obj_t screen;

  Menu root;
  auto submenu = std::make_unique<Menu>("Submenu");
  submenu->add_child(std::make_unique<ActionWidget>("Sub Item"));
  Menu* submenu_ptr = submenu.get();

  root.add_child(std::make_unique<SubmenuWidget>("Settings", std::move(submenu)));
  root.add_child(std::make_unique<ActionWidget>("Back"));

  MenuController controller(&screen);

  controller.push(&root);
  TEST_ASSERT(controller.current_menu() == &root);

  // Long press on SubmenuWidget should push the submenu.
  controller.handle_input(false);
  TEST_ASSERT(controller.stack_depth() == 2);
  TEST_ASSERT(controller.current_menu() == submenu_ptr);
}

// ============================================================================
// Integration Tests
// ============================================================================

void test_full_menu_workflow() {
  lv_obj_t screen;

  // Build a menu structure.
  Menu root;

  // Protocol submenu.
  auto protocol_menu = std::make_unique<Menu>("Protocol");
  bool bias_changed = false;
  auto bias_action = std::make_unique<ActionWidget>("Bias: +0us");
  bias_action->set_on_selected([&bias_changed](ActionWidget& w) {
    bias_changed = true;
    w.set_label("Bias: +10us");
  });
  protocol_menu->add_child(std::move(bias_action));
  protocol_menu->add_child(std::make_unique<ActionWidget>("Back"));

  root.add_child(
      std::make_unique<SubmenuWidget>("Protocol", std::move(protocol_menu)));

  bool system_selected = false;
  auto system_action = std::make_unique<ActionWidget>("System");
  system_action->set_on_selected([&system_selected](ActionWidget&) {
    system_selected = true;
  });
  root.add_child(std::move(system_action));

  root.add_child(std::make_unique<ActionWidget>("Back"));

  // Start navigation.
  MenuController controller(&screen);
  controller.push(&root);
  TEST_ASSERT(controller.get_focus_index() == 0);

  // Select Protocol submenu.
  controller.handle_input(false);  // Long press.
  TEST_ASSERT(controller.stack_depth() == 2);

  // Select Bias action.
  controller.handle_input(false);  // Long press on Bias.
  TEST_ASSERT(bias_changed);

  // Navigate to System in root (pop first).
  controller.pop();
  controller.focus_next();  // to System.
  TEST_ASSERT(controller.get_focus_index() == 1);

  controller.handle_input(false);  // Select System.
  TEST_ASSERT(system_selected);
}

// ============================================================================
// SpinnerWidget Tests
// ============================================================================

void test_spinner_widget_creation_with_external_ptr() {
  int32_t value = 50;
  SpinnerWidget widget(&value, 0, 100, 10);

  TEST_ASSERT(widget.get_value() == 50);
  TEST_ASSERT(widget.get_min() == 0);
  TEST_ASSERT(widget.get_max() == 100);
  TEST_ASSERT(widget.get_step() == 10);
  TEST_ASSERT(!widget.is_editing());
  TEST_ASSERT(widget.is_focusable());
}

void test_spinner_widget_creation_with_internal_value() {
  SpinnerWidget widget(25, 0, 100, 5);

  TEST_ASSERT(widget.get_value() == 25);
  TEST_ASSERT(widget.get_min() == 0);
  TEST_ASSERT(widget.get_max() == 100);
  TEST_ASSERT(widget.get_step() == 5);
}

void test_spinner_widget_set_value() {
  SpinnerWidget widget(0, 0, 100, 10);

  widget.set_value(75);
  TEST_ASSERT(widget.get_value() == 75);
}

void test_spinner_widget_external_ptr_modifies_original() {
  int32_t value = 50;
  SpinnerWidget widget(&value, 0, 100, 10);

  widget.set_value(75);
  TEST_ASSERT(value == 75);  // Original variable modified.
}

void test_spinner_widget_short_press_not_editing() {
  SpinnerWidget widget(50, 0, 100, 10);

  // Short press when not editing should move to next.
  InputResult result = widget.handle_input(true);
  TEST_ASSERT(result == InputResult::kFocusNext);
}

void test_spinner_widget_long_press_enters_edit_mode() {
  SpinnerWidget widget(50, 0, 100, 10);

  TEST_ASSERT(!widget.is_editing());

  // Long press enters edit mode.
  InputResult result = widget.handle_input(false);
  TEST_ASSERT(result == InputResult::kSelected);
  TEST_ASSERT(widget.is_editing());
}

void test_spinner_widget_short_press_while_editing_cycles() {
  int32_t value = 50;
  SpinnerWidget widget(&value, 0, 100, 10);

  // Enter edit mode.
  widget.handle_input(false);
  TEST_ASSERT(widget.is_editing());

  // Short press cycles value.
  InputResult result = widget.handle_input(true);
  TEST_ASSERT(result == InputResult::kConsumed);
  TEST_ASSERT(value == 60);

  // Cycle again.
  widget.handle_input(true);
  TEST_ASSERT(value == 70);
}

void test_spinner_widget_long_press_while_editing_commits() {
  SpinnerWidget widget(50, 0, 100, 10);

  bool commit_called = false;
  int32_t committed_value = 0;
  widget.set_on_commit([&](SpinnerWidget&, int32_t v) {
    commit_called = true;
    committed_value = v;
  });

  // Enter edit mode.
  widget.handle_input(false);
  TEST_ASSERT(widget.is_editing());

  // Cycle once.
  widget.handle_input(true);
  TEST_ASSERT(widget.get_value() == 60);

  // Commit with long press.
  InputResult result = widget.handle_input(false);
  TEST_ASSERT(result == InputResult::kDeselected);
  TEST_ASSERT(!widget.is_editing());
  TEST_ASSERT(commit_called);
  TEST_ASSERT(committed_value == 60);
}

void test_spinner_widget_value_wraps_at_max() {
  int32_t value = 90;
  SpinnerWidget widget(&value, 0, 100, 10);

  // Enter edit mode.
  widget.handle_input(false);

  // Cycle past max.
  widget.handle_input(true);  // 100
  TEST_ASSERT(value == 100);

  widget.handle_input(true);  // wraps to 0.
  TEST_ASSERT(value == 0);
}

void test_spinner_widget_on_changed_callback() {
  int32_t value = 50;
  SpinnerWidget widget(&value, 0, 100, 10);

  int change_count = 0;
  int32_t last_changed_value = 0;
  widget.set_on_changed([&](SpinnerWidget&, int32_t v) {
    change_count++;
    last_changed_value = v;
  });

  // Enter edit mode.
  widget.handle_input(false);

  // Cycle triggers on_changed.
  widget.handle_input(true);
  TEST_ASSERT(change_count == 1);
  TEST_ASSERT(last_changed_value == 60);

  widget.handle_input(true);
  TEST_ASSERT(change_count == 2);
  TEST_ASSERT(last_changed_value == 70);
}

void test_spinner_widget_presets() {
  int32_t value = 100;
  SpinnerWidget widget(&value, 0, 0, 0);  // min/max/step ignored for presets.

  static const int32_t presets[] = {50, 100, 250, 500, 1000};
  widget.set_presets(presets, 5);

  // Enter edit mode.
  widget.handle_input(false);

  // Should start at index matching current value (100 = index 1).
  // Cycle to next preset.
  widget.handle_input(true);
  TEST_ASSERT(value == 250);

  widget.handle_input(true);
  TEST_ASSERT(value == 500);

  widget.handle_input(true);
  TEST_ASSERT(value == 1000);

  widget.handle_input(true);  // Wraps.
  TEST_ASSERT(value == 50);
}

void test_spinner_widget_format_callback() {
  SpinnerWidget widget(50, 0, 100, 10);

  bool format_called = false;
  widget.set_format([&](int32_t v, char* buf, size_t sz) {
    format_called = true;
    snprintf(buf, sz, "Value: %d%%", static_cast<int>(v));
  });

  // Render triggers format.
  lv_obj_t parent;
  widget.render(&parent, 0);

  TEST_ASSERT(format_called);
}

void test_spinner_widget_focus_loss_commits() {
  SpinnerWidget widget(50, 0, 100, 10);

  bool commit_called = false;
  widget.set_on_commit([&](SpinnerWidget&, int32_t) {
    commit_called = true;
  });

  // First set focused.
  widget.set_focused(true);
  TEST_ASSERT(widget.is_focused());

  // Enter edit mode.
  widget.handle_input(false);
  TEST_ASSERT(widget.is_editing());

  // Cycle once.
  widget.handle_input(true);

  // Lose focus should commit.
  widget.set_focused(false);
  TEST_ASSERT(!widget.is_editing());
  TEST_ASSERT(commit_called);
}

// ============================================================================
// ToggleWidget Tests
// ============================================================================

void test_toggle_widget_creation_with_external_ptr() {
  bool value = true;
  ToggleWidget widget("Test Toggle", &value);

  TEST_ASSERT(strcmp(widget.get_label(), "Test Toggle") == 0);
  TEST_ASSERT(widget.get_value() == true);
  TEST_ASSERT(widget.is_focusable());
}

void test_toggle_widget_creation_with_internal_value() {
  ToggleWidget widget("Test Toggle", false);

  TEST_ASSERT(widget.get_value() == false);
}

void test_toggle_widget_set_value() {
  ToggleWidget widget("Test", false);

  widget.set_value(true);
  TEST_ASSERT(widget.get_value() == true);

  widget.set_value(false);
  TEST_ASSERT(widget.get_value() == false);
}

void test_toggle_widget_external_ptr_modifies_original() {
  bool value = false;
  ToggleWidget widget("Test", &value);

  widget.set_value(true);
  TEST_ASSERT(value == true);  // Original variable modified.
}

void test_toggle_widget_short_press() {
  ToggleWidget widget("Test", false);

  // Short press should move to next item.
  InputResult result = widget.handle_input(true);
  TEST_ASSERT(result == InputResult::kFocusNext);
  TEST_ASSERT(widget.get_value() == false);  // Value unchanged.
}

void test_toggle_widget_long_press_toggles() {
  bool value = false;
  ToggleWidget widget("Test", &value);

  // Long press toggles.
  InputResult result = widget.handle_input(false);
  TEST_ASSERT(result == InputResult::kConsumed);
  TEST_ASSERT(value == true);

  // Toggle again.
  widget.handle_input(false);
  TEST_ASSERT(value == false);
}

void test_toggle_widget_on_changed_callback() {
  bool value = false;
  ToggleWidget widget("Test", &value);

  int change_count = 0;
  bool last_state = false;
  widget.set_on_changed([&](ToggleWidget&, bool state) {
    change_count++;
    last_state = state;
  });

  widget.handle_input(false);
  TEST_ASSERT(change_count == 1);
  TEST_ASSERT(last_state == true);

  widget.handle_input(false);
  TEST_ASSERT(change_count == 2);
  TEST_ASSERT(last_state == false);
}

void test_toggle_widget_callback_receives_widget_reference() {
  ToggleWidget widget("Test", false);

  ToggleWidget* received_widget = nullptr;
  widget.set_on_changed([&](ToggleWidget& w, bool) {
    received_widget = &w;
  });

  widget.handle_input(false);
  TEST_ASSERT(received_widget == &widget);
}

void test_toggle_widget_custom_state_text() {
  ToggleWidget widget("Feature", true);
  widget.set_state_text("Enabled", "Disabled");

  // Verify the method exists and widget still functions.
  TEST_ASSERT(widget.get_value() == true);
}

void test_toggle_widget_focus() {
  ToggleWidget widget("Test", false);

  TEST_ASSERT(!widget.is_focused());

  widget.set_focused(true);
  TEST_ASSERT(widget.is_focused());

  widget.set_focused(false);
  TEST_ASSERT(!widget.is_focused());
}

}  // namespace

int run_tests() {
  printf("\n=== Widget System Unit Tests ===\n\n");

  // ActionWidget tests.
  RUN_TEST(test_action_widget_creation);
  RUN_TEST(test_action_widget_focus);
  RUN_TEST(test_action_widget_input_short_press);
  RUN_TEST(test_action_widget_input_long_press);
  RUN_TEST(test_action_widget_callback_receives_widget_reference);
  RUN_TEST(test_action_widget_set_label);

  // Menu tests.
  RUN_TEST(test_menu_creation);
  RUN_TEST(test_menu_creation_no_title);
  RUN_TEST(test_menu_add_children);
  RUN_TEST(test_menu_get_child);
  RUN_TEST(test_menu_get_focusable_child);
  RUN_TEST(test_menu_is_not_focusable);

  // MenuController tests.
  RUN_TEST(test_controller_creation);
  RUN_TEST(test_controller_push_menu);
  RUN_TEST(test_controller_push_multiple_menus);
  RUN_TEST(test_controller_pop_menu);
  RUN_TEST(test_controller_cannot_pop_last_menu);
  RUN_TEST(test_controller_clear);
  RUN_TEST(test_controller_focus_starts_at_zero);
  RUN_TEST(test_controller_focus_next);
  RUN_TEST(test_controller_focus_prev);
  RUN_TEST(test_controller_set_focus);
  RUN_TEST(test_controller_get_focused_widget);
  RUN_TEST(test_controller_handle_input_routes_to_widget);
  RUN_TEST(test_controller_short_press_moves_focus);
  RUN_TEST(test_controller_focus_restored_on_pop);

  // SubmenuWidget tests.
  RUN_TEST(test_submenu_widget_creation);
  RUN_TEST(test_submenu_widget_input);
  RUN_TEST(test_controller_submenu_navigation);

  // Integration tests.
  RUN_TEST(test_full_menu_workflow);

  // SpinnerWidget tests.
  RUN_TEST(test_spinner_widget_creation_with_external_ptr);
  RUN_TEST(test_spinner_widget_creation_with_internal_value);
  RUN_TEST(test_spinner_widget_set_value);
  RUN_TEST(test_spinner_widget_external_ptr_modifies_original);
  RUN_TEST(test_spinner_widget_short_press_not_editing);
  RUN_TEST(test_spinner_widget_long_press_enters_edit_mode);
  RUN_TEST(test_spinner_widget_short_press_while_editing_cycles);
  RUN_TEST(test_spinner_widget_long_press_while_editing_commits);
  RUN_TEST(test_spinner_widget_value_wraps_at_max);
  RUN_TEST(test_spinner_widget_on_changed_callback);
  RUN_TEST(test_spinner_widget_presets);
  RUN_TEST(test_spinner_widget_format_callback);
  RUN_TEST(test_spinner_widget_focus_loss_commits);

  // ToggleWidget tests.
  RUN_TEST(test_toggle_widget_creation_with_external_ptr);
  RUN_TEST(test_toggle_widget_creation_with_internal_value);
  RUN_TEST(test_toggle_widget_set_value);
  RUN_TEST(test_toggle_widget_external_ptr_modifies_original);
  RUN_TEST(test_toggle_widget_short_press);
  RUN_TEST(test_toggle_widget_long_press_toggles);
  RUN_TEST(test_toggle_widget_on_changed_callback);
  RUN_TEST(test_toggle_widget_callback_receives_widget_reference);
  RUN_TEST(test_toggle_widget_custom_state_text);
  RUN_TEST(test_toggle_widget_focus);

  printf("\n=== Results: %d passed, %d failed ===\n\n",
         tests_passed, tests_failed);

  return tests_failed;
}
