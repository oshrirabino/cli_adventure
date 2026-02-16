cmake_minimum_required(VERSION 3.16)
project(cli_adventure LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(adventure_engine
    src/context/game_context.cpp
    src/engine/engine.cpp
    src/levels/choice_level.cpp
    src/levels/end_game_level.cpp
    src/levels/input_level.cpp
    src/levels/terminal_level_factory.cpp
    src/parser/tag_parser.cpp
    src/ui/renderer.cpp
    src/ui/terminal_menu.cpp
    src/ui/theme.cpp
    src/validation/game_validator.cpp
)

target_include_directories(adventure_engine PUBLIC src)

if(MSVC)
    target_compile_options(adventure_engine PRIVATE /W4 /permissive-)
else()
    target_compile_options(adventure_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(cli_adventure src/main.cpp)
target_link_libraries(cli_adventure PRIVATE adventure_engine)

include(CTest)
if(BUILD_TESTING)
    add_executable(context_tests tests/context_tests.cpp)
    target_link_libraries(context_tests PRIVATE adventure_engine)
    add_test(NAME context_tests COMMAND context_tests)

    add_executable(parser_tests tests/parser_tests.cpp)
    target_link_libraries(parser_tests PRIVATE adventure_engine)
    add_test(NAME parser_tests COMMAND parser_tests)

    add_executable(choice_level_tests tests/choice_level_tests.cpp)
    target_link_libraries(choice_level_tests PRIVATE adventure_engine)
    add_test(NAME choice_level_tests COMMAND choice_level_tests)

    add_executable(input_level_tests tests/input_level_tests.cpp)
    target_link_libraries(input_level_tests PRIVATE adventure_engine)
    add_test(NAME input_level_tests COMMAND input_level_tests)

    add_executable(engine_flow_tests tests/engine_flow_tests.cpp)
    target_link_libraries(engine_flow_tests PRIVATE adventure_engine)
    add_test(NAME engine_flow_tests COMMAND engine_flow_tests)
endif()
