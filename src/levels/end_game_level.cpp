#include "levels/end_game_level.h"

#include <utility>

namespace adventure::levels {
EndGameLevel::EndGameLevel(adventure::parser::ParsedLevelData data,
                           const adventure::ui::Renderer& renderer)
    : title_(build_title(data.header)),
      ascii_art_path_(data.header.count("ascii_art") != 0 ? data.header.at("ascii_art") : ""),
      content_lines_(std::move(data.content_lines)),
      directives_(std::move(data.directives)),
      renderer_(renderer) {}

void EndGameLevel::render(std::ostream& out,
                          const adventure::context::GameContext& context) const {
  renderer_.render_scene(out, title_, content_lines_, context.current_directory(), ascii_art_path_);
}

void EndGameLevel::execute(std::istream& in, std::ostream& out,
                           adventure::context::GameContext& context) {
  (void)in;
  const auto result_it = directives_.find("result");
  if (result_it == directives_.end()) {
    renderer_.render_structure_error(out,
                                     "EndGame level must define `result: victory|game_over`.");
    context.set_game_over(true);
    return;
  }

  if (result_it->second == "victory") {
    renderer_.render_victory(out);
    context.set_victory(true);
    return;
  }

  if (result_it->second == "game_over") {
    renderer_.render_game_over(out);
    context.set_game_over(true);
    return;
  }

  renderer_.render_structure_error(out, "Invalid endgame result `" + result_it->second +
                                           "`. Expected `victory` or `game_over`.");
  context.set_game_over(true);
}

std::string EndGameLevel::build_title(
    const std::unordered_map<std::string, std::string>& header) {
  const auto title_it = header.find("title");
  if (title_it != header.end() && !title_it->second.empty()) {
    return title_it->second;
  }
  return "End Game";
}

}  // namespace adventure::levels
