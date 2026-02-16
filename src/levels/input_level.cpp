#include "levels/input_level.h"

#include <algorithm>
#include <cctype>
#include <exception>
#include <string>
#include <utility>

#include "ui/terminal_menu.h"

namespace adventure::levels {
namespace {

bool parse_bool(const std::string& value) {
  std::string normalized;
  normalized.reserve(value.size());
  for (char c : value) {
    normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
  }
  return normalized == "true" || normalized == "1" || normalized == "yes" || normalized == "on";
}

}  // namespace

InputLevel::InputLevel(adventure::parser::ParsedLevelData data,
                       const adventure::ui::Renderer& renderer)
    : title_(build_title(data.header)),
      ascii_art_path_(data.header.count("ascii_art") != 0 ? data.header.at("ascii_art") : ""),
      content_lines_(std::move(data.content_lines)),
      input_rules_(std::move(data.input_rules)),
      on_enter_memory_(std::move(data.on_enter_memory)),
      option_conditions_(std::move(data.option_conditions)),
      option_effects_(std::move(data.option_effects)),
      renderer_(renderer) {
  for (std::size_t i = 0; i < input_rules_.size(); ++i) {
    input_rules_[i].id = resolve_rule_id(input_rules_[i], i);
    rule_ids_.insert(input_rules_[i].id);
  }

  if (data.directives.find("input_prompt") != data.directives.end()) {
    input_prompt_ = data.directives.at("input_prompt");
  }
  if (data.directives.find("input_invalid_message") != data.directives.end()) {
    input_invalid_message_ = data.directives.at("input_invalid_message");
  }
  if (data.directives.find("input_match") != data.directives.end()) {
    input_match_mode_ = data.directives.at("input_match");
  }
  if (data.directives.find("input_case_sensitive") != data.directives.end()) {
    input_case_sensitive_ = parse_bool(data.directives.at("input_case_sensitive"));
  }
}

void InputLevel::render(std::ostream& out,
                        const adventure::context::GameContext& context) const {
  renderer_.render_scene(out, title_, content_lines_, context.current_directory(), ascii_art_path_);
}

void InputLevel::execute(std::istream& in, std::ostream& out,
                         adventure::context::GameContext& context) {
  apply_mutations(on_enter_memory_, context);
  const bool is_interactive = adventure::ui::supports_interactive_menu(in, out);
  std::size_t transient_lines = 1;  // initial prompt line

  std::string invalid_rule_id;
  if (!validate_rule_ids(&invalid_rule_id)) {
    context.set_game_over(true);
    renderer_.render_structure_error(out,
                                     "Unknown rule id in condition/effect: `" + invalid_rule_id +
                                         "`.");
    return;
  }

  if (input_rules_.empty()) {
    context.set_game_over(true);
    renderer_.render_structure_error(out, "Input level has no INPUT_RULES.");
    return;
  }

  out << "\n" << input_prompt_ << " ";
  std::string user_input;
  while (std::getline(in, user_input)) {
    bool matched = false;
    for (const auto& rule : input_rules_) {
      if (!conditions_pass_for_rule(rule.id, context)) {
        continue;
      }
      if (!is_rule_match(rule, user_input)) {
        continue;
      }

      for (const auto& effect : option_effects_) {
        if (effect.option_id == rule.id) {
          apply_mutations(effect.mutations, context);
        }
      }

      if (is_interactive) {
        renderer_.clear_last_scene(out, transient_lines);
      }
      context.request_next_level(rule.target);
      matched = true;
      break;
    }

    if (matched) {
      return;
    }

    out << input_invalid_message_ << "\n" << input_prompt_ << " ";
    transient_lines += 1;  // invalid message prints one full line
  }

  context.set_game_over(true);
}

std::string InputLevel::build_title(const std::unordered_map<std::string, std::string>& header) {
  const auto title_it = header.find("title");
  if (title_it != header.end() && !title_it->second.empty()) {
    return title_it->second;
  }

  const auto id_it = header.find("id");
  if (id_it != header.end() && !id_it->second.empty()) {
    return "Level: " + id_it->second;
  }
  return "Input Level";
}

std::string InputLevel::resolve_rule_id(const adventure::parser::InputRule& rule,
                                        std::size_t index) {
  if (!rule.id.empty()) {
    return rule.id;
  }
  return "rule_" + std::to_string(index + 1);
}

std::string InputLevel::normalize_input(std::string value, bool case_sensitive) {
  if (case_sensitive) {
    return value;
  }
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return value;
}

bool InputLevel::conditions_pass_for_rule(
    const std::string& rule_id, const adventure::context::GameContext& context) const {
  for (const auto& condition : option_conditions_) {
    if (condition.option_id != rule_id) {
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

void InputLevel::apply_mutations(const std::vector<adventure::parser::MemoryMutation>& mutations,
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

bool InputLevel::validate_rule_ids(std::string* invalid_rule_id) const {
  for (const auto& condition : option_conditions_) {
    if (rule_ids_.find(condition.option_id) == rule_ids_.end()) {
      *invalid_rule_id = condition.option_id;
      return false;
    }
  }
  for (const auto& effect : option_effects_) {
    if (rule_ids_.find(effect.option_id) == rule_ids_.end()) {
      *invalid_rule_id = effect.option_id;
      return false;
    }
  }
  return true;
}

bool InputLevel::is_rule_match(const adventure::parser::InputRule& rule,
                               const std::string& user_input) const {
  const std::string normalized_input = normalize_input(user_input, input_case_sensitive_);
  const std::string normalized_pattern = normalize_input(rule.pattern, input_case_sensitive_);

  if (input_match_mode_ == "exact") {
    return normalized_input == normalized_pattern;
  }
  if (input_match_mode_ == "prefix") {
    return normalized_input.rfind(normalized_pattern, 0) == 0;
  }

  return normalized_input.find(normalized_pattern) != std::string::npos;
}

}  // namespace adventure::levels
