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
      on_enter_memory_(std::move(data.on_enter_memory)),
      option_conditions_(std::move(data.option_conditions)),
      option_effects_(std::move(data.option_effects)),
      renderer_(renderer) {
  for (std::size_t i = 0; i < options_.size(); ++i) {
    options_[i].id = resolve_option_id(options_[i], i);
    option_ids_.insert(options_[i].id);
  }
}

void ChoiceLevel::render(std::ostream& out,
                         const adventure::context::GameContext& context) const {
  renderer_.render_scene(out, title_, content_lines_, context.current_directory(), ascii_art_path_);
}

void ChoiceLevel::execute(std::istream& in, std::ostream& out,
                          adventure::context::GameContext& context) {
  apply_mutations(on_enter_memory_, context);

  std::string invalid_option_id;
  if (!validate_rule_option_ids(&invalid_option_id)) {
    context.set_game_over(true);
    renderer_.render_structure_error(
        out, "Unknown option id in rule: `" + invalid_option_id + "`.");
    return;
  }

  if (options_.empty()) {
    context.set_game_over(true);
    renderer_.render_structure_error(out, "Choice level has no options.");
    return;
  }

  std::vector<std::size_t> visible_option_indices;
  std::vector<std::string> labels;
  labels.reserve(options_.size());
  for (std::size_t index = 0; index < options_.size(); ++index) {
    const auto& option = options_[index];
    if (!conditions_pass_for_option(option.id, context)) {
      continue;
    }
    visible_option_indices.push_back(index);
    labels.push_back(option.text);
  }

  if (labels.empty()) {
    context.set_game_over(true);
    renderer_.render_structure_error(
        out, "No visible options after evaluating memory conditions.");
    return;
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
    const std::size_t resolved_index = visible_option_indices[selection.index];
    const auto& selected_option = options_[resolved_index];

    for (const auto& effect : option_effects_) {
      if (effect.option_id == selected_option.id) {
        apply_mutations(effect.mutations, context);
      }
    }

    context.request_next_level(selected_option.target);
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

std::string ChoiceLevel::resolve_option_id(const adventure::parser::LevelOption& option,
                                           std::size_t index) {
  if (!option.id.empty()) {
    return option.id;
  }
  return "option_" + std::to_string(index + 1);
}

bool ChoiceLevel::conditions_pass_for_option(
    const std::string& option_id, const adventure::context::GameContext& context) const {
  for (const auto& condition : option_conditions_) {
    if (condition.option_id != option_id) {
      continue;
    }

    for (const auto& flag : condition.required_flags) {
      if (!context.has_memory_flag(flag)) {
        return false;
      }
    }

    for (const auto& flag : condition.forbidden_flags) {
      if (context.has_memory_flag(flag)) {
        return false;
      }
    }

    for (const auto& requirement : condition.required_values) {
      const std::string* value = context.get_memory_value(requirement.first);
      if (value == nullptr || *value != requirement.second) {
        return false;
      }
    }

    for (const auto& missing_key : condition.required_missing_values) {
      if (context.has_memory_value(missing_key)) {
        return false;
      }
    }
  }

  return true;
}

void ChoiceLevel::apply_mutations(
    const std::vector<adventure::parser::MemoryMutation>& mutations,
    adventure::context::GameContext& context) const {
  for (const auto& mutation : mutations) {
    switch (mutation.kind) {
      case adventure::parser::MemoryMutation::Kind::kAddFlag:
        context.set_memory_flag(mutation.key);
        break;
      case adventure::parser::MemoryMutation::Kind::kClearFlag:
        context.clear_memory_flag(mutation.key);
        break;
      case adventure::parser::MemoryMutation::Kind::kSetValue:
        context.set_memory_value(mutation.key, mutation.value);
        break;
      case adventure::parser::MemoryMutation::Kind::kEraseValue:
        context.erase_memory_value(mutation.key);
        break;
    }
  }
}

bool ChoiceLevel::validate_rule_option_ids(std::string* invalid_option_id) const {
  for (const auto& condition : option_conditions_) {
    if (option_ids_.find(condition.option_id) == option_ids_.end()) {
      *invalid_option_id = condition.option_id;
      return false;
    }
  }

  for (const auto& effect : option_effects_) {
    if (option_ids_.find(effect.option_id) == option_ids_.end()) {
      *invalid_option_id = effect.option_id;
      return false;
    }
  }

  return true;
}

}  // namespace adventure::levels
