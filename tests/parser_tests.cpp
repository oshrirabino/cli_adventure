#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "parser/tag_parser.h"

namespace {

void expect(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << "\n";
    std::exit(1);
  }
}

void test_happy_path() {
  const std::string input = R"(
[HEADER]
id: intro
title: The First Room

[CONTENT]
You wake in a locked chamber.
There is a wooden door to the north.

[OPTIONS]
Inspect the door -> ./door.level
Shout for help => ../hall/help.level

[DIRECTIVES]
ascii: assets/room.txt
input_mode: choice
)";

  adventure::parser::TagParser parser;
  std::istringstream stream(input);
  const adventure::parser::ParsedLevelData data = parser.parse(stream);

  expect(data.header.at("id") == "intro", "Header id should be parsed.");
  expect(data.header.at("title") == "The First Room", "Header title should be parsed.");
  expect(data.content_lines.size() == 3, "Content should preserve 3 lines including blank line.");
  expect(data.options.size() == 2, "Two options should be parsed.");
  expect(data.options[0].id.empty(), "Option id should be empty when omitted.");
  expect(data.options[0].text == "Inspect the door", "First option text mismatch.");
  expect(data.options[0].target == "./door.level", "First option target mismatch.");
  expect(data.options[1].text == "Shout for help", "Second option text mismatch.");
  expect(data.options[1].target == "../hall/help.level", "Second option target mismatch.");
  expect(data.directives.at("ascii") == "assets/room.txt", "Directive ascii mismatch.");
  expect(data.directives.at("input_mode") == "choice", "Directive input_mode mismatch.");
}

void test_malformed_lines_are_ignored() {
  const std::string input = R"(
[HEADER]
missing separator
ok: yes

[OPTIONS]
bad option format
Open chest ->
Take torch -> ./torch.level

[DIRECTIVES]
another bad line
difficulty: easy
)";

  adventure::parser::TagParser parser;
  std::istringstream stream(input);
  const adventure::parser::ParsedLevelData data = parser.parse(stream);

  expect(data.header.size() == 1, "Malformed header lines should be ignored.");
  expect(data.header.at("ok") == "yes", "Valid header line should be parsed.");
  expect(data.options.size() == 1, "Only one valid option should be parsed.");
  expect(data.options[0].text == "Take torch", "Valid option text mismatch.");
  expect(data.options[0].target == "./torch.level", "Valid option target mismatch.");
  expect(data.directives.size() == 1, "Malformed directives should be ignored.");
  expect(data.directives.at("difficulty") == "easy", "Valid directive mismatch.");
}

void test_missing_sections() {
  const std::string input = R"(
This text appears before any section and should be ignored.
[CONTENT]
Single content line.
)";

  adventure::parser::TagParser parser;
  std::istringstream stream(input);
  const adventure::parser::ParsedLevelData data = parser.parse(stream);

  expect(data.header.empty(), "Header should be empty when section is missing.");
  expect(data.options.empty(), "Options should be empty when section is missing.");
  expect(data.directives.empty(), "Directives should be empty when section is missing.");
  expect(data.content_lines.size() == 1, "Content line should be parsed.");
  expect(data.content_lines[0] == "Single content line.", "Content text mismatch.");
}

void test_option_uses_first_delimiter_position() {
  const std::string input = R"(
[OPTIONS]
Read sign => clue -> ./target.level
)";

  adventure::parser::TagParser parser;
  std::istringstream stream(input);
  const adventure::parser::ParsedLevelData data = parser.parse(stream);

  expect(data.options.size() == 1, "Expected one parsed option.");
  expect(data.options[0].text == "Read sign", "Option should split at first delimiter position.");
  expect(data.options[0].target == "clue -> ./target.level",
         "Option target should keep remaining text after first delimiter.");
}

void test_option_with_explicit_id() {
  const std::string input = R"(
[OPTIONS]
cut_tree | Cut the tree down -> ./clearing.level
)";

  adventure::parser::TagParser parser;
  std::istringstream stream(input);
  const adventure::parser::ParsedLevelData data = parser.parse(stream);

  expect(data.options.size() == 1, "Expected one parsed option.");
  expect(data.options[0].id == "cut_tree", "Explicit option id should be parsed.");
  expect(data.options[0].text == "Cut the tree down", "Option text mismatch.");
  expect(data.options[0].target == "./clearing.level", "Option target mismatch.");
}

}  // namespace

int main() {
  test_happy_path();
  test_malformed_lines_are_ignored();
  test_missing_sections();
  test_option_uses_first_delimiter_position();
  test_option_with_explicit_id();
  return 0;
}
