#ifndef CLI_ADVENTURE_LEVELS_ANIMATED_LEVEL_DECORATOR_H_
#define CLI_ADVENTURE_LEVELS_ANIMATED_LEVEL_DECORATOR_H_

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "levels/ilevel.h"
#include "parser/parsed_level.h"
#include "ui/renderer.h"

namespace adventure::levels {

class AnimatedLevelDecorator final : public ILevel {
 public:
  AnimatedLevelDecorator(std::unique_ptr<ILevel> wrapped,
                         adventure::parser::ParsedLevelData data,
                         const adventure::ui::Renderer& renderer);

  void render(std::ostream& out,
              const adventure::context::GameContext& context) const override;
  void execute(std::istream& in, std::ostream& out,
               adventure::context::GameContext& context) override;

 private:
  struct AnimationData {
    int fps = 12;
    std::vector<std::vector<std::string>> frames;
  };

  static std::string build_title(const adventure::parser::ParsedLevelData& data);
  static bool parse_animation_file(const std::filesystem::path& path,
                                   AnimationData* animation);
  static std::string trim(std::string value);
  static std::string to_lower(std::string value);
  static std::string colorize(const std::string& text, const adventure::ui::Theme& theme,
                              const std::string& color_name);
  static void render_scene_with_frame(std::ostream& out, const adventure::ui::Theme& theme,
                                      const std::string& title,
                                      const std::vector<std::string>& content_lines,
                                      const std::vector<std::string>& frame_lines,
                                      std::size_t max_frame_lines);

  std::unique_ptr<ILevel> wrapped_;
  std::string title_;
  std::string ascii_art_path_;
  std::vector<std::string> content_lines_;
  const adventure::ui::Renderer& renderer_;
};

}  // namespace adventure::levels

#endif  // CLI_ADVENTURE_LEVELS_ANIMATED_LEVEL_DECORATOR_H_
