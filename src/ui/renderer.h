#ifndef CLI_ADVENTURE_UI_RENDERER_H_
#define CLI_ADVENTURE_UI_RENDERER_H_

#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

#include "ui/theme.h"

namespace adventure::ui {

class Renderer {
 public:
  explicit Renderer(Theme theme);

  const Theme& theme() const;

  void render_scene(std::ostream& out, const std::string& title,
                    const std::vector<std::string>& content_lines,
                    const std::string& current_directory,
                    const std::string& ascii_art_relative_path) const;
  void clear_last_scene(std::ostream& out, std::size_t extra_lines_after_scene = 0) const;

  void render_victory(std::ostream& out) const;
  void render_game_over(std::ostream& out) const;
  void render_structure_error(std::ostream& out, const std::string& message) const;

 private:
  struct AsciiArtLine {
    std::string text;
    std::string color_name;
  };

  static bool parse_ascii_art_default_color_directive(const std::string& raw_line,
                                                      std::string* color_name);
  static AsciiArtLine parse_ascii_art_line(const std::string& raw_line);
  std::string colorize(const std::string& text, const std::string& color_name) const;
  std::vector<AsciiArtLine> load_ascii_art(const std::string& current_directory,
                                           const std::string& ascii_art_relative_path) const;

  mutable std::size_t last_scene_lines_ = 0;
  Theme theme_;
};

}  // namespace adventure::ui

#endif  // CLI_ADVENTURE_UI_RENDERER_H_
