#include "levels/end_game_level.h"

#include <utility>

#include "ui/terminal_menu.h"

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
  const bool is_interactive = adventure::ui::supports_interactive_menu(in, out);
  const std::string continue_prompt = "Press Enter to return to Main Menu...";
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
    adventure::ui::wait_for_continue(in, out, continue_prompt);
    if (is_interactive) {
      // Clear scene + victory banner + continue prompt lines.
      renderer_.clear_last_scene(out, 4);
    }
    return;
  }

  if (result_it->second == "game_over") {
    renderer_.render_game_over(out);
    context.set_game_over(true);
    adventure::ui::wait_for_continue(in, out, continue_prompt);
    if (is_interactive) {
      // Clear scene + game-over banner + continue prompt lines.
      renderer_.clear_last_scene(out, 4);
    }
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
