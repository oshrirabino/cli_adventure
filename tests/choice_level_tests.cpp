#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "context/game_context.h"
#include "levels/choice_level.h"
#include "parser/parsed_level.h"
#include "ui/renderer.h"
#include "ui/theme.h"

namespace {

void expect(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << "\n";
    std::exit(1);
  }
}

void test_invalid_then_valid_choice() {
  adventure::parser::ParsedLevelData data;
  data.header["title"] = "Entry Hall";
  data.content_lines = {"Pick your direction."};
  data.options = {{"Go left", "./left.level"}, {"Go right", "./right.level"}};
  data.directives["input_mode"] = "choice";

  adventure::ui::Renderer renderer(adventure::ui::Theme{});
  adventure::levels::ChoiceLevel level(data, renderer);
  adventure::context::GameContext context;

  std::istringstream input("banana\n99\n2\n");
  std::ostringstream output;

  level.render(output, context);
  level.execute(input, output, context);

  expect(context.has_next_level_request(), "A valid choice should request the next level.");
  expect(context.next_level_request() == "./right.level", "Selected target mismatch.");

  const std::string transcript = output.str();
  expect(transcript.find("Invalid selection") != std::string::npos,
         "Invalid feedback prompt should appear.");
}

void test_choice_level_without_options_is_structure_error() {
  adventure::parser::ParsedLevelData data;
  data.header["title"] = "Broken Level";
  data.content_lines = {"This level forgot to define options."};

  adventure::ui::Renderer renderer(adventure::ui::Theme{});
  adventure::levels::ChoiceLevel level(data, renderer);
  adventure::context::GameContext context;

  std::istringstream input("");
  std::ostringstream output;

  level.render(output, context);
  level.execute(input, output, context);

  expect(context.is_game_over(), "Choice level without options must end as structure error.");
  expect(!context.is_victory(), "Structure error should not mark victory.");
  expect(output.str().find("[Structure Error] Choice level has no options.") != std::string::npos,
         "Structure error message should be printed for empty choice levels.");
}

}  // namespace

int main() {
  test_invalid_then_valid_choice();
  test_choice_level_without_options_is_structure_error();
  return 0;
}
