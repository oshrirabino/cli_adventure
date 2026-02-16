#include "parser/tag_parser.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace adventure::parser {
namespace {

enum class Section {
  kNone,
  kHeader,
  kContent,
  kOptions,
  kDirectives,
  kMemory,
  kOptionConditions,
  kOptionEffects,
};

std::string trim(std::string value) {
  const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
    return std::isspace(ch) != 0;
  });
  if (first == value.end()) {
    return "";
  }
  const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
    return std::isspace(ch) != 0;
  }).base();
  return std::string(first, last);
}

std::string to_upper(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
    return static_cast<char>(std::toupper(ch));
  });
  return value;
}

std::string to_lower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return value;
}

bool is_section_tag(const std::string& line, std::string* section_name) {
  const std::string candidate = trim(line);
  if (candidate.size() < 3 || candidate.front() != '[' || candidate.back() != ']') {
    return false;
  }
  *section_name = to_upper(trim(candidate.substr(1, candidate.size() - 2)));
  return !section_name->empty();
}

Section section_from_name(const std::string& section_name) {
  if (section_name == "HEADER") {
    return Section::kHeader;
  }
  if (section_name == "CONTENT") {
    return Section::kContent;
  }
  if (section_name == "OPTIONS") {
    return Section::kOptions;
  }
  if (section_name == "DIRECTIVES") {
    return Section::kDirectives;
  }
  if (section_name == "MEMORY") {
    return Section::kMemory;
  }
  if (section_name == "OPTION_CONDITIONS") {
    return Section::kOptionConditions;
  }
  if (section_name == "OPTION_EFFECTS") {
    return Section::kOptionEffects;
  }
  return Section::kNone;
}

bool split_kv(const std::string& line, char delimiter, std::string* left, std::string* right) {
  const auto pos = line.find(delimiter);
  if (pos == std::string::npos) {
    return false;
  }
  *left = trim(line.substr(0, pos));
  *right = trim(line.substr(pos + 1));
  return !left->empty() && !right->empty();
}

bool split_option(const std::string& line, std::string* id, std::string* text, std::string* target) {
  const auto arrow = line.find("->");
  const auto fat_arrow = line.find("=>");

  std::size_t pos = std::string::npos;
  std::size_t separator_size = 0;

  if (arrow != std::string::npos &&
      (fat_arrow == std::string::npos || arrow < fat_arrow)) {
    pos = arrow;
    separator_size = 2;
  } else if (fat_arrow != std::string::npos) {
    pos = fat_arrow;
    separator_size = 2;
  } else {
    return false;
  }

  const std::string option_decl = trim(line.substr(0, pos));
  *target = trim(line.substr(pos + separator_size));

  const auto pipe_pos = option_decl.find('|');
  if (pipe_pos != std::string::npos) {
    *id = trim(option_decl.substr(0, pipe_pos));
    *text = trim(option_decl.substr(pipe_pos + 1));
    return !id->empty() && !text->empty() && !target->empty();
  }

  id->clear();
  *text = option_decl;
  return !text->empty() && !target->empty();
}

std::vector<std::string> split_tokens(const std::string& line) {
  std::istringstream in(line);
  std::vector<std::string> tokens;
  std::string token;
  while (in >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

bool split_token_kv(const std::string& token, std::string* key, std::string* value) {
  const auto eq = token.find('=');
  if (eq == std::string::npos) {
    return false;
  }
  *key = trim(token.substr(0, eq));
  *value = trim(token.substr(eq + 1));
  return !key->empty() && !value->empty();
}

bool parse_colon_pair(const std::string& token, std::string* left, std::string* right) {
  const auto pos = token.find(':');
  if (pos == std::string::npos) {
    return false;
  }
  *left = trim(token.substr(0, pos));
  *right = trim(token.substr(pos + 1));
  return !left->empty() && !right->empty();
}

bool parse_mutation_token(const std::string& key, const std::string& value,
                          adventure::parser::MemoryMutation* mutation) {
  if (key == "add_flag") {
    mutation->kind = adventure::parser::MemoryMutation::Kind::kAddFlag;
    mutation->key = value;
    mutation->value.clear();
    return true;
  }
  if (key == "clear_flag") {
    mutation->kind = adventure::parser::MemoryMutation::Kind::kClearFlag;
    mutation->key = value;
    mutation->value.clear();
    return true;
  }
  if (key == "erase_value") {
    mutation->kind = adventure::parser::MemoryMutation::Kind::kEraseValue;
    mutation->key = value;
    mutation->value.clear();
    return true;
  }
  if (key == "set_value") {
    std::string memory_key;
    std::string memory_value;
    if (!parse_colon_pair(value, &memory_key, &memory_value)) {
      return false;
    }
    mutation->kind = adventure::parser::MemoryMutation::Kind::kSetValue;
    mutation->key = memory_key;
    mutation->value = memory_value;
    return true;
  }
  return false;
}

void parse_memory_line(const std::string& line,
                       std::vector<adventure::parser::MemoryMutation>* out) {
  const std::vector<std::string> tokens = split_tokens(line);
  bool is_on_enter = false;

  for (const std::string& token : tokens) {
    if (to_upper(token) == "ON_ENTER") {
      is_on_enter = true;
      continue;
    }
    std::string key;
    std::string value;
    if (!split_token_kv(token, &key, &value)) {
      continue;
    }
    adventure::parser::MemoryMutation mutation;
    if (parse_mutation_token(to_lower(key), value, &mutation) && is_on_enter) {
      out->push_back(std::move(mutation));
    }
  }
}

void parse_option_condition_line(const std::string& line,
                                 std::vector<adventure::parser::OptionCondition>* out) {
  const std::vector<std::string> tokens = split_tokens(line);
  adventure::parser::OptionCondition condition;

  for (const std::string& token : tokens) {
    std::string key;
    std::string value;
    if (!split_token_kv(token, &key, &value)) {
      continue;
    }
    const std::string normalized_key = to_lower(key);
    if (normalized_key == "option") {
      condition.option_id = value;
    } else if (normalized_key == "requires_flag") {
      condition.required_flags.push_back(value);
    } else if (normalized_key == "forbids_flag") {
      condition.forbidden_flags.push_back(value);
    } else if (normalized_key == "requires_value") {
      std::string memory_key;
      std::string memory_value;
      if (parse_colon_pair(value, &memory_key, &memory_value)) {
        condition.required_values.push_back({memory_key, memory_value});
      }
    } else if (normalized_key == "requires_missing_value") {
      condition.required_missing_values.push_back(value);
    }
  }

  if (!condition.option_id.empty()) {
    out->push_back(std::move(condition));
  }
}

void parse_option_effect_line(const std::string& line,
                              std::vector<adventure::parser::OptionEffect>* out) {
  const std::vector<std::string> tokens = split_tokens(line);
  adventure::parser::OptionEffect effect;

  for (const std::string& token : tokens) {
    std::string key;
    std::string value;
    if (!split_token_kv(token, &key, &value)) {
      continue;
    }
    const std::string normalized_key = to_lower(key);
    if (normalized_key == "option") {
      effect.option_id = value;
      continue;
    }

    adventure::parser::MemoryMutation mutation;
    if (parse_mutation_token(normalized_key, value, &mutation)) {
      effect.mutations.push_back(std::move(mutation));
    }
  }

  if (!effect.option_id.empty() && !effect.mutations.empty()) {
    out->push_back(std::move(effect));
  }
}

}  // namespace

ParsedLevelData TagParser::parse(std::istream& input) const {
  ParsedLevelData data;
  Section section = Section::kNone;

  std::string raw_line;
  while (std::getline(input, raw_line)) {
    std::string section_name;
    if (is_section_tag(raw_line, &section_name)) {
      section = section_from_name(section_name);
      continue;
    }

    const std::string line = trim(raw_line);
    if (line.empty()) {
      if (section == Section::kContent) {
        data.content_lines.emplace_back("");
      }
      continue;
    }

    switch (section) {
      case Section::kHeader: {
        std::string key;
        std::string value;
        if (split_kv(line, ':', &key, &value)) {
          data.header[key] = value;
        }
        break;
      }
      case Section::kContent:
        data.content_lines.push_back(raw_line);
        break;
      case Section::kOptions: {
        std::string id;
        std::string text;
        std::string target;
        if (split_option(line, &id, &text, &target)) {
          data.options.push_back(LevelOption{id, text, target});
        }
        break;
      }
      case Section::kDirectives: {
        std::string key;
        std::string value;
        if (split_kv(line, ':', &key, &value)) {
          data.directives[key] = value;
        }
        break;
      }
      case Section::kMemory:
        parse_memory_line(line, &data.on_enter_memory);
        break;
      case Section::kOptionConditions:
        parse_option_condition_line(line, &data.option_conditions);
        break;
      case Section::kOptionEffects:
        parse_option_effect_line(line, &data.option_effects);
        break;
      case Section::kNone:
        break;
    }
  }

  return data;
}

ParsedLevelData TagParser::parse_file(const std::filesystem::path& file_path) const {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open level file: " + file_path.string());
  }
  return parse(file);
}

}  // namespace adventure::parser
