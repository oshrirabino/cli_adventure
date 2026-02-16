#include "levels/choice_level.h"

#include <cstddef>
#include <exception>
#include <iostream>
#include <utility>
#include <vector>

#include "ui/terminal_menu.h"

namespace adventure::levels {

ChoiceLevel::ChoiceLevel(adventure::parser::ParsedLevelData data,
                         const adventure::ui::Renderer& renderer)
    : title_(build_title(data.header)),
      ascii_art_path_(data.header.count("ascii_art") != 0 ? data.header.at("ascii_art") : ""),
      content_lines_(std::move(data.content_lines)),
      options_(std::move(data.options)),
      renderer_(renderer) {}

void ChoiceLevel::render(std::ostream& out,
                         const adventure::context::GameContext& context) const {
  renderer_.render_scene(out, title_, content_lines_, context.current_directory(), ascii_art_path_);
}

void ChoiceLevel::execute(std::istream& in, std::ostream& out,
                          adventure::context::GameContext& context) {
  if (options_.empty()) {
    context.set_game_over(true);
    renderer_.render_structure_error(out, "Choice level has no options.");
    return;
  }

  std::vector<std::string> labels;
  labels.reserve(options_.size());
  for (const auto& option : options_) {
    labels.push_back(option.text);
  }

  const bool is_interactive = adventure::ui::supports_interactive_menu(in, out);
  const std::string prompt =
      is_interactive
          ? "Use Up/Down arrows and Enter to choose:"
          : "Interactive menu unavailable; use number input:";

  try {
    const adventure::ui::MenuSelection selection =
        adventure::ui::pick_option(in, out, labels, prompt, renderer_.theme());
    if (is_interactive) {
      renderer_.clear_last_scene(out, selection.rendered_lines);
    }
    context.request_next_level(options_[selection.index].target);
    return;
  } catch (const std::exception&) {
    context.set_game_over(true);
  }
}

std::string ChoiceLevel::build_title(
    const std::unordered_map<std::string, std::string>& header) {
  const auto title_it = header.find("title");
  if (title_it != header.end() && !title_it->second.empty()) {
    return title_it->second;
  }

  const auto id_it = header.find("id");
  if (id_it != header.end() && !id_it->second.empty()) {
    return "Level: " + id_it->second;
  }

  return "Untitled Level";
}

}  // namespace adventure::levels
