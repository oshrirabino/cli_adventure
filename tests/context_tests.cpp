#include <cstdlib>
#include <iostream>
#include <string>

#include "context/game_context.h"

namespace {

void expect(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << "\n";
    std::exit(1);
  }
}

}  // namespace

int main() {
  adventure::context::GameContext context;

  expect(!context.is_game_over(), "Game should not start in game-over state.");
  expect(!context.is_victory(), "Game should not start in victory state.");
  expect(context.memory_values().empty(), "Memory key/value map should start empty.");
  expect(context.memory_flags().empty(), "Memory flag set should start empty.");

  context.set_current_directory("campaign/start");
  context.set_current_level_path("campaign/start/intro.level");
  context.request_next_level("../village/market.level");
  context.set_memory_value("player.health", "83");
  context.set_memory_value("item.rusty-key", "owned");
  context.set_memory_flag("door.opened");
  context.set_game_over(true);
  context.set_victory(true);

  expect(context.current_directory() == "campaign/start", "Current directory write/read mismatch.");
  expect(context.current_level_path() == "campaign/start/intro.level",
         "Current level path write/read mismatch.");
  expect(context.has_next_level_request(), "Next-level request should be set.");
  expect(context.next_level_request() == "../village/market.level",
         "Next-level request value mismatch.");
  expect(context.has_memory_value("player.health"), "Memory value key should exist.");
  expect(context.has_memory_value("item.rusty-key"), "Item memory key should exist.");
  expect(context.get_memory_value("player.health") != nullptr,
         "Memory getter should return a value pointer.");
  expect(*context.get_memory_value("player.health") == "83", "Memory value content mismatch.");
  expect(context.has_memory_flag("door.opened"), "Memory flag should exist.");
  expect(context.is_game_over(), "Game-over flag write/read mismatch.");
  expect(context.is_victory(), "Victory flag write/read mismatch.");

  context.erase_memory_value("item.rusty-key");
  context.clear_memory_flag("door.opened");
  context.clear_next_level_request();

  expect(!context.has_memory_value("item.rusty-key"), "Memory value should be erasable.");
  expect(!context.has_memory_flag("door.opened"), "Memory flag should be erasable.");
  expect(!context.has_next_level_request(), "Next-level request should be cleared.");

  return 0;
}
