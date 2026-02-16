#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "context/game_context.h"
#include "levels/input_level.h"
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

void test_input_rule_match_and_transition() {
  adventure::parser::ParsedLevelData data;
  data.header["title"] = "Riddle Door";
  data.directives["input_mode"] = "input";
  data.directives["input_prompt"] = "Speak:";
  data.input_rules = {{"say_friend", "friend", "./open.level"}};

  adventure::ui::Renderer renderer(adventure::ui::Theme{});
  adventure::levels::InputLevel level(data, renderer);
  adventure::context::GameContext context;

  std::istringstream input("nothing\nsay friend please\n");
  std::ostringstream output;

  level.render(output, context);
  level.execute(input, output, context);

  expect(context.has_next_level_request(), "Matching input rule should request next level.");
  expect(context.next_level_request() == "./open.level", "Input rule target mismatch.");
}

void test_input_memory_condition_and_effect() {
  adventure::parser::ParsedLevelData data;
  data.header["title"] = "Demon Chamber";
  data.directives["input_mode"] = "input";
  data.input_rules = {{"fight_magic", "fight demon", "./win.level"},
                      {"fight_plain", "fight demon", "./lose.level"}};
  data.option_conditions.push_back(
      adventure::parser::OptionCondition{"fight_magic", {"got_magic_sword"}, {}, {}, {}});
  data.option_conditions.push_back(
      adventure::parser::OptionCondition{"fight_plain", {}, {"got_magic_sword"}, {}, {}});
  data.option_effects.push_back(adventure::parser::OptionEffect{
      "fight_magic",
      {adventure::parser::MemoryMutation{
          adventure::parser::MemoryMutation::Kind::kSetValue, "demon.outcome", "slain"}}});

  adventure::ui::Renderer renderer(adventure::ui::Theme{});
  adventure::levels::InputLevel level(data, renderer);
  adventure::context::GameContext context;
  context.set_memory_flag("got_magic_sword");

  std::istringstream input("fight demon\n");
  std::ostringstream output;
  level.render(output, context);
  level.execute(input, output, context);

  expect(context.has_next_level_request(), "Input should route when conditioned rule is visible.");
  expect(context.next_level_request() == "./win.level", "Expected magic path target.");
  expect(context.get_memory_value("demon.outcome") != nullptr,
         "Option effect should write memory value.");
  expect(*context.get_memory_value("demon.outcome") == "slain",
         "Option effect memory value mismatch.");
}

}  // namespace

int main() {
  test_input_rule_match_and_transition();
  test_input_memory_condition_and_effect();
  return 0;
}
