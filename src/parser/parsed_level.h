#ifndef CLI_ADVENTURE_PARSER_PARSED_LEVEL_H_
#define CLI_ADVENTURE_PARSER_PARSED_LEVEL_H_

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace adventure::parser {

struct LevelOption {
  std::string id;
  std::string text;
  std::string target;
};

struct InputRule {
  std::string id;
  std::string pattern;
  std::string target;
};

struct MemoryMutation {
  enum class Kind {
    kAddFlag,
    kClearFlag,
    kSetValue,
    kEraseValue,
  };

  Kind kind;
  std::string key;
  std::string value;
};

struct OptionCondition {
  std::string option_id;
  std::vector<std::string> required_flags;
  std::vector<std::string> forbidden_flags;
  std::vector<std::pair<std::string, std::string>> required_values;
  std::vector<std::string> required_missing_values;
};

struct OptionEffect {
  std::string option_id;
  std::vector<MemoryMutation> mutations;
};

struct ParsedLevelData {
  std::unordered_map<std::string, std::string> header;
  std::vector<std::string> content_lines;
  std::vector<LevelOption> options;
  std::vector<InputRule> input_rules;
  std::unordered_map<std::string, std::string> directives;
  std::vector<MemoryMutation> on_enter_memory;
  std::vector<OptionCondition> option_conditions;
  std::vector<OptionEffect> option_effects;
};

}  // namespace adventure::parser

#endif  // CLI_ADVENTURE_PARSER_PARSED_LEVEL_H_
