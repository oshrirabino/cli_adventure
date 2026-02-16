#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "context/game_context.h"
#include "engine/engine.h"

namespace {

void expect(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << "\n";
    std::exit(1);
  }
}

void write_text_file(const std::filesystem::path& path, const std::string& content) {
  std::ofstream out(path);
  if (!out.is_open()) {
    std::cerr << "FAILED: cannot write " << path << "\n";
    std::exit(1);
  }
  out << content;
}

void test_engine_navigates_to_next_level() {
  const std::filesystem::path root =
      std::filesystem::temp_directory_path() / "cli_adventure_engine_flow_tests";
  std::filesystem::create_directories(root);

  const std::filesystem::path start_level = root / "start.level";
  const std::filesystem::path win_level = root / "win.level";

  write_text_file(start_level, R"([HEADER]
title: Start

[CONTENT]
A fork in the road.

[OPTIONS]
Take the safe path -> ./win.level

[DIRECTIVES]
input_mode: choice
)");

  write_text_file(win_level, R"([HEADER]
title: Win

[CONTENT]
You made it.

[DIRECTIVES]
input_mode: endgame
result: victory
)");

  adventure::context::GameContext context;
  context.set_current_level_path(start_level.string());

  adventure::engine::Engine engine;
  std::istringstream input("1\n");
  std::ostringstream output;
  engine.run(input, output, context);

  expect(context.is_victory(), "Engine should reach victory terminal level.");
  expect(context.current_level_path() == win_level.lexically_normal().string(),
         "Engine should update current level path after transition.");
}

void test_engine_handles_missing_next_level_file() {
  const std::filesystem::path root =
      std::filesystem::temp_directory_path() / "cli_adventure_engine_flow_missing_file_tests";
  std::filesystem::create_directories(root);

  const std::filesystem::path start_level = root / "start.level";
  write_text_file(start_level, R"([HEADER]
title: Start

[CONTENT]
Broken path ahead.

[OPTIONS]
Go forward -> ./missing.level

[DIRECTIVES]
input_mode: choice
)");

  adventure::context::GameContext context;
  context.set_current_level_path(start_level.string());

  adventure::engine::Engine engine;
  std::istringstream input("1\n");
  std::ostringstream output;
  engine.run(input, output, context);

  expect(context.is_game_over(), "Engine should terminate with game_over on missing file.");
  expect(!context.is_victory(), "Missing file path must not trigger victory.");
  expect(output.str().find("[Structure Error]") != std::string::npos,
         "Missing file should produce a structure error message.");
}

void test_engine_memory_condition_unlocks_option() {
  const std::filesystem::path root =
      std::filesystem::temp_directory_path() / "cli_adventure_engine_flow_memory_tests";
  std::filesystem::create_directories(root);

  const std::filesystem::path start_level = root / "start.level";
  const std::filesystem::path stash_level = root / "stash.level";
  const std::filesystem::path win_level = root / "win.level";

  write_text_file(start_level, R"([HEADER]
title: Start

[CONTENT]
A locked gate blocks the path.

[OPTIONS]
search | Search the room -> ./stash.level
open_gate | Open the gate -> ./win.level

[DIRECTIVES]
input_mode: choice

[OPTION_CONDITIONS]
option=open_gate requires_flag=got_key
)");

  write_text_file(stash_level, R"([HEADER]
title: Stash

[CONTENT]
You find a key.

[OPTIONS]
take_key | Take key and return -> ./start.level

[DIRECTIVES]
input_mode: choice

[OPTION_EFFECTS]
option=take_key add_flag=got_key
)");

  write_text_file(win_level, R"([HEADER]
title: Win

[CONTENT]
The gate opens.

[DIRECTIVES]
input_mode: endgame
result: victory
)");

  adventure::context::GameContext context;
  context.set_current_level_path(start_level.string());

  adventure::engine::Engine engine;
  std::istringstream input("1\n1\n2\n");
  std::ostringstream output;
  engine.run(input, output, context);

  expect(context.is_victory(), "Memory-gated option should unlock and reach victory.");
}

}  // namespace

int main() {
  test_engine_navigates_to_next_level();
  test_engine_handles_missing_next_level_file();
  test_engine_memory_condition_unlocks_option();
  return 0;
}
