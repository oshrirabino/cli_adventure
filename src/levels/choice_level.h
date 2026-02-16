#ifndef CLI_ADVENTURE_LEVELS_CHOICE_LEVEL_H_
#define CLI_ADVENTURE_LEVELS_CHOICE_LEVEL_H_

#include <string>
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

  std::string title_;
  std::string ascii_art_path_;
  std::vector<std::string> content_lines_;
  std::vector<adventure::parser::LevelOption> options_;
  const adventure::ui::Renderer& renderer_;
};

}  // namespace adventure::levels

#endif  // CLI_ADVENTURE_LEVELS_CHOICE_LEVEL_H_
