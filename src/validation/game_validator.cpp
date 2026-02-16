#include "validation/game_validator.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <unordered_set>
#include <vector>

#include "parser/parsed_level.h"
#include "parser/tag_parser.h"

namespace adventure::validation {
namespace {

std::string resolve_option_id(const adventure::parser::LevelOption& option, std::size_t index) {
  if (!option.id.empty()) {
    return option.id;
  }
  return "option_" + std::to_string(index + 1);
}

std::string resolve_rule_id(const adventure::parser::InputRule& rule, std::size_t index) {
  if (!rule.id.empty()) {
    return rule.id;
  }
  return "rule_" + std::to_string(index + 1);
}

bool file_exists(const std::filesystem::path& path) {
  return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
}

}  // namespace

ValidationReport validate_game(const std::filesystem::path& game_root) {
  ValidationReport report;
  adventure::parser::TagParser parser;

  std::vector<std::filesystem::path> levels;
  for (const auto& entry : std::filesystem::recursive_directory_iterator(game_root)) {
    if (!entry.is_regular_file() || entry.path().extension() != ".level") {
      continue;
    }
    levels.push_back(entry.path().lexically_normal());
  }

  std::sort(levels.begin(), levels.end());

  for (const auto& level_path : levels) {
    ++report.checked_files;

    adventure::parser::ParsedLevelData data;
    try {
      data = parser.parse_file(level_path);
    } catch (const std::exception& ex) {
      report.issues.push_back({level_path, "Parse failure: " + std::string(ex.what())});
      continue;
    }

    const std::string input_mode =
        (data.directives.find("input_mode") != data.directives.end())
            ? data.directives.at("input_mode")
            : "choice";
    const bool is_endgame = input_mode == "endgame";

    if (is_endgame) {
      const auto result_it = data.directives.find("result");
      if (result_it == data.directives.end() ||
          (result_it->second != "victory" && result_it->second != "game_over")) {
        report.issues.push_back(
            {level_path, "Endgame level must define `result: victory|game_over`."});
      }
      continue;
    }

    std::unordered_set<std::string> option_ids;
    if (input_mode == "input") {
      if (data.input_rules.empty()) {
        report.issues.push_back({level_path, "Input level has no INPUT_RULES."});
        continue;
      }
      for (std::size_t i = 0; i < data.input_rules.size(); ++i) {
        const std::string rule_id = resolve_rule_id(data.input_rules[i], i);
        if (!option_ids.insert(rule_id).second) {
          report.issues.push_back({level_path, "Duplicate input rule id: `" + rule_id + "`."});
        }

        const std::filesystem::path target = data.input_rules[i].target;
        const std::filesystem::path resolved =
            target.is_absolute() ? target.lexically_normal()
                                 : (level_path.parent_path() / target).lexically_normal();
        if (!file_exists(resolved)) {
          report.issues.push_back(
              {level_path, "Missing input rule target `" + data.input_rules[i].target + "`."});
        }
      }
    } else {
      if (data.options.empty()) {
        report.issues.push_back({level_path, "Choice level has no options."});
        continue;
      }
      for (std::size_t i = 0; i < data.options.size(); ++i) {
        const std::string option_id = resolve_option_id(data.options[i], i);
        if (!option_ids.insert(option_id).second) {
          report.issues.push_back({level_path, "Duplicate option id: `" + option_id + "`."});
        }

        const std::filesystem::path target = data.options[i].target;
        const std::filesystem::path resolved =
            target.is_absolute() ? target.lexically_normal()
                                 : (level_path.parent_path() / target).lexically_normal();
        if (!file_exists(resolved)) {
          report.issues.push_back(
              {level_path, "Missing option target `" + data.options[i].target + "`."});
        }
      }
    }

    for (const auto& condition : data.option_conditions) {
      if (option_ids.find(condition.option_id) == option_ids.end()) {
        report.issues.push_back(
            {level_path, "OPTION_CONDITIONS references unknown option id `" + condition.option_id + "`."});
      }
    }

    for (const auto& effect : data.option_effects) {
      if (option_ids.find(effect.option_id) == option_ids.end()) {
        report.issues.push_back(
            {level_path, "OPTION_EFFECTS references unknown option id `" + effect.option_id + "`."});
      }
    }
  }

  const std::filesystem::path start_file = (game_root / "start.level").lexically_normal();
  if (!file_exists(start_file)) {
    report.issues.push_back({start_file, "Game root must contain start.level."});
  }

  return report;
}

}  // namespace adventure::validation
