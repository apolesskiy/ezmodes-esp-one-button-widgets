cmake_minimum_required(VERSION 3.16)
project(ezmodes_widgets_host_tests C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# Stub ESP-IDF / LVGL headers
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/stubs
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

add_executable(test_widgets
  test_widgets.cpp
  test_main.cpp
  # Source files under test
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/menu.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/menu_controller.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/widgets/action_widget.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/widgets/submenu_widget.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/widgets/spinner_widget.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/widgets/toggle_widget.cpp
)

add_test(NAME widget_tests COMMAND test_widgets)
