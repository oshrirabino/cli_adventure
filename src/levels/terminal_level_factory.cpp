#include "levels/terminal_level_factory.h"

#include <algorithm>
#include <cctype>
#include <string>

#include "levels/animated_level_decorator.h"
#include "levels/choice_level.h"
#include "levels/end_game_level.h"
#include "levels/input_level.h"

namespace adventure::levels {
namespace {

std::string to_lower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return value;
}

bool should_use_animated_decorator(const adventure::parser::ParsedLevelData& data) {
  const auto mode_it = data.directives.find("ascii_art_mode");
  if (mode_it == data.directives.end()) {
    return false;
  }
  if (data.header.find("ascii_art") == data.header.end() || data.header.at("ascii_art").empty()) {
    return false;
  }
  return to_lower(mode_it->second) == "animated";
}

}  // namespace

TerminalLevelFactory::TerminalLevelFactory(const adventure::ui::Renderer& renderer)
    : renderer_(renderer) {}

std::unique_ptr<ILevel> TerminalLevelFactory::create(
    const adventure::parser::ParsedLevelData& data) const {
  adventure::parser::ParsedLevelData level_data = data;
  const auto mode_it = data.directives.find("input_mode");
  std::unique_ptr<ILevel> base_level;
  if (mode_it != data.directives.end() && mode_it->second == "endgame") {
    base_level = std::make_unique<EndGameLevel>(std::move(level_data), renderer_);
  } else if (mode_it != data.directives.end() && mode_it->second == "input") {
    base_level = std::make_unique<InputLevel>(std::move(level_data), renderer_);
  } else {
    base_level = std::make_unique<ChoiceLevel>(std::move(level_data), renderer_);
  }

  if (should_use_animated_decorator(data)) {
    return std::make_unique<AnimatedLevelDecorator>(
        std::move(base_level), adventure::parser::ParsedLevelData(data), renderer_);
  }

  return base_level;
}

}  // namespace adventure::levels
