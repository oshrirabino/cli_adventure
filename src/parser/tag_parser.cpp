#include "parser/tag_parser.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <string>

namespace adventure::parser {
namespace {

enum class Section {
  kNone,
  kHeader,
  kContent,
  kOptions,
  kDirectives,
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

bool split_option(const std::string& line, std::string* text, std::string* target) {
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

  *text = trim(line.substr(0, pos));
  *target = trim(line.substr(pos + separator_size));
  return !text->empty() && !target->empty();
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
        std::string text;
        std::string target;
        if (split_option(line, &text, &target)) {
          data.options.push_back(LevelOption{text, target});
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
