## Technology Stack
* The project uses esp-idf v5.5.x for ESP32 targets.
* The project language is C++17.
* UI framework: LVGL via `esp_lvgl_port`.
* Depends on `ezmodes_esp_base` for `ModalFramework`, `ModeInterface`, and `ButtonEvent`.

## Testing
* Host-based tests run on the development machine using assert-based test harness.
* LVGL and ESP-IDF functions are stubbed for host compilation.
* Run tests: `cd host_test && cmake -B build && cmake --build build && ctest --output-on-failure`
* On-device tests require LVGL display initialization.

## Agent-First Development
* Documentation should be readable by humans, but optimized for consumption by AI agents.
* Tools used should support agent interaction (such as MCP integration).
* In cases where a frequently used tool does not support smooth agent interaction, an interaction layer should be created. This can be a separate task.
* Code should be written with AI agents in mind, without compromising readability and editability for humans.
* In projects with components/tasks that are physical or otherwise inaccessible to AI agents, provide clear instructions or documentation for completion of the task by a human. Ensure bidirectional communication and that assumptions are checked.
* Avoid including code in design documents.

## ESP-IDF Development
* This project uses ESP-IDF v5.5.x for ESP32-S3.
* Before running any `idf.py` commands, source the ESP-IDF environment: `. $IDF_PATH/export.sh`
* Build command: `idf.py build`
* Flash command: `idf.py -p <PORT> flash` (requires physical connection to target device)
* Monitor command: `idf.py -p <PORT> monitor` (view serial output)
* Clean build: `idf.py fullclean`
* Set target: `idf.py set-target esp32s3`
* Configure: `idf.py menuconfig` (interactive) or edit `sdkconfig.defaults`

### Component Development
* Follow esp-idf component architecture. Each reusable module should be a separate component in `components/`.
* Components should have their own `CMakeLists.txt` and `idf_component.yml` for dependency management.
* Internal components use relative paths. External components should be registered in the ESP Component Registry when mature.
* Component public headers go in `include/` subdirectory.

## Style
* Use spaces for indentation. Indentation width is 2 spaces.
* For a given language, determine style as follows:
  1. If present, the style guide document at `agent/style/<language>.md` takes precedence.
  2. Officially recommended/adopted style decisions, such as PEP8 for Python and CamelCase naming for C#.
  3. The most common or widely accepted style guide for the language.

### C++ Style (Primary Language)
* Follow Google C++ Style Guide with ESP-IDF modifications.
* Use `.cpp` extension for C++ source files, `.hpp` for C++ headers.
* Naming conventions:
  - Classes/Structs: `PascalCase`
  - Functions/Methods: `snake_case` (esp-idf convention)
  - Variables: `snake_case`
  - Constants/Macros: `UPPER_SNAKE_CASE`
  - Private members: `snake_case_` (trailing underscore)
* Use `#pragma once` for header guards.
* Prefer `constexpr` over `#define` for constants.

## Code Structure
* Strongly prefer small, iterative code changes.
* Keep function/method length under 50 lines where possible.
* Each function/method in non-test code must have a documentation comment.

## Testing
* Host-based tests (running on development machine) are preferred for logic that doesn't require hardware.
* Test command: `cd host_test && cmake -B build && cmake --build build && ctest --test-dir build --output-on-failure`

## Progress Checkpoints
* Create an intermediate commit for every minimally committable change.
* Each commit's description should start with "[Agent]".
* Each commit message should clearly describe the change made.
