#ifndef CLI_ADVENTURE_PARSER_TAG_PARSER_H_
#define CLI_ADVENTURE_PARSER_TAG_PARSER_H_

#include <filesystem>
#include <istream>

#include "parser/parsed_level.h"

namespace adventure::parser {

class TagParser {
 public:
  ParsedLevelData parse(std::istream& input) const;
  ParsedLevelData parse_file(const std::filesystem::path& file_path) const;
};

}  // namespace adventure::parser

#endif  // CLI_ADVENTURE_PARSER_TAG_PARSER_H_
