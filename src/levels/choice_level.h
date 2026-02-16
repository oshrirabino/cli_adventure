#ifndef CLI_ADVENTURE_LEVELS_CHOICE_LEVEL_H_
#define CLI_ADVENTURE_LEVELS_CHOICE_LEVEL_H_

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "levels/ilevel.h"
#include "parser/parsed_level.h"
#include "ui/renderer.h"

namespace adventure::levels {

class ChoiceLevel final : public ILevel {
 public:
  ChoiceLevel(adventure::parser::ParsedLevelData data, const adventure::ui::Renderer& renderer);

  void render(std::ostream& out,
              const adventure::context::GameContext& context) const override;
  void execute(std::istream& in, std::ostream& out,
               adventure::context::GameContext& context) override;

 private:
  static std::string build_title(
      const std::unordered_map<std::string, std::string>& header);
  static std::string resolve_option_id(const adventure::parser::LevelOption& option,
                                       std::size_t index);
  bool conditions_pass_for_option(const std::string& option_id,
                                  const adventure::context::GameContext& context) const;
  void apply_mutations(const std::vector<adventure::parser::MemoryMutation>& mutations,
                       adventure::context::GameContext& context) const;
  bool validate_rule_option_ids(std::string* invalid_option_id) const;

  std::string title_;
  std::string ascii_art_path_;
  std::vector<std::string> content_lines_;
  std::vector<adventure::parser::LevelOption> options_;
  std::vector<adventure::parser::MemoryMutation> on_enter_memory_;
  std::vector<adventure::parser::OptionCondition> option_conditions_;
  std::vector<adventure::parser::OptionEffect> option_effects_;
  std::unordered_set<std::string> option_ids_;
  const adventure::ui::Renderer& renderer_;
};

}  // namespace adventure::levels

#endif  // CLI_ADVENTURE_LEVELS_CHOICE_LEVEL_H_
