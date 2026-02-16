#include "levels/terminal_level_factory.h"

#include "levels/choice_level.h"
#include "levels/end_game_level.h"
#include "levels/input_level.h"

namespace adventure::levels {

TerminalLevelFactory::TerminalLevelFactory(const adventure::ui::Renderer& renderer)
    : renderer_(renderer) {}

std::unique_ptr<ILevel> TerminalLevelFactory::create(
    const adventure::parser::ParsedLevelData& data) const {
  const auto mode_it = data.directives.find("input_mode");
  if (mode_it != data.directives.end() && mode_it->second == "endgame") {
    return std::make_unique<EndGameLevel>(data, renderer_);
  }
  if (mode_it != data.directives.end() && mode_it->second == "input") {
    return std::make_unique<InputLevel>(data, renderer_);
  }

  if (mode_it == data.directives.end() || mode_it->second == "choice") {
    return std::make_unique<ChoiceLevel>(data, renderer_);
  }

  return std::make_unique<ChoiceLevel>(data, renderer_);
}

}  // namespace adventure::levels
