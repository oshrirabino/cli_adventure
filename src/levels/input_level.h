#ifndef CLI_ADVENTURE_LEVELS_INPUT_LEVEL_H_
#define CLI_ADVENTURE_LEVELS_INPUT_LEVEL_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "levels/ilevel.h"
#include "parser/parsed_level.h"
#include "ui/renderer.h"

namespace adventure::levels {

class InputLevel final : public ILevel {
 public:
  InputLevel(adventure::parser::ParsedLevelData data, const adventure::ui::Renderer& renderer);

  void render(std::ostream& out,
              const adventure::context::GameContext& context) const override;
  void execute(std::istream& in, std::ostream& out,
               adventure::context::GameContext& context) override;

 private:
  static std::string build_title(
      const std::unordered_map<std::string, std::string>& header);
  static std::string resolve_rule_id(const adventure::parser::InputRule& rule, std::size_t index);
  static std::string normalize_input(std::string value, bool case_sensitive);

  bool conditions_pass_for_rule(const std::string& rule_id,
                                const adventure::context::GameContext& context) const;
  void apply_mutations(const std::vector<adventure::parser::MemoryMutation>& mutations,
                       adventure::context::GameContext& context) const;
  bool validate_rule_ids(std::string* invalid_rule_id) const;
  bool is_rule_match(const adventure::parser::InputRule& rule, const std::string& user_input) const;

  std::string title_;
  std::string ascii_art_path_;
  std::vector<std::string> content_lines_;
  std::vector<adventure::parser::InputRule> input_rules_;
  std::vector<adventure::parser::MemoryMutation> on_enter_memory_;
  std::vector<adventure::parser::OptionCondition> option_conditions_;
  std::vector<adventure::parser::OptionEffect> option_effects_;
  std::unordered_set<std::string> rule_ids_;
  std::string input_prompt_ = "What do you do?";
  std::string input_invalid_message_ = "Nothing happens. Try again.";
  std::string input_match_mode_ = "contains";
  bool input_case_sensitive_ = false;
  const adventure::ui::Renderer& renderer_;
};

}  // namespace adventure::levels

#endif  // CLI_ADVENTURE_LEVELS_INPUT_LEVEL_H_
