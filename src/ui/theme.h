#ifndef CLI_ADVENTURE_UI_THEME_H_
#define CLI_ADVENTURE_UI_THEME_H_

#include <filesystem>
#include <string>

namespace adventure::ui {

struct Theme {
  std::string border_line = "============================================================";
  std::string menu_selected_prefix = "->";
  std::string menu_unselected_prefix = ">";

  std::string title_color = "bright_cyan";
  std::string body_color = "default";
  std::string prompt_color = "bright_yellow";
  std::string selected_color = "bright_green";
  std::string unselected_color = "default";
  std::string victory_color = "bright_green";
  std::string game_over_color = "bright_red";
  std::string error_color = "bright_red";

  bool use_color = false;
};

Theme load_theme_from_file(const std::filesystem::path& theme_file, const Theme& base_theme);
bool is_color_enabled_for_terminal();
std::string ansi_color_code(const std::string& color_name);

}  // namespace adventure::ui

#endif  // CLI_ADVENTURE_UI_THEME_H_
