#ifndef CLI_ADVENTURE_PARSER_PARSED_LEVEL_H_
#define CLI_ADVENTURE_PARSER_PARSED_LEVEL_H_

#include <string>
#include <unordered_map>
#include <vector>

namespace adventure::parser {

struct LevelOption {
  std::string text;
  std::string target;
};

struct ParsedLevelData {
  std::unordered_map<std::string, std::string> header;
  std::vector<std::string> content_lines;
  std::vector<LevelOption> options;
  std::unordered_map<std::string, std::string> directives;
};

}  // namespace adventure::parser

#endif  // CLI_ADVENTURE_PARSER_PARSED_LEVEL_H_
