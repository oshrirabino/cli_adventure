#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "ui/renderer.h"
#include "ui/theme.h"

namespace {

void expect(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << "\n";
    std::exit(1);
  }
}

void write_text_file(const std::filesystem::path& path, const std::vector<std::string>& lines) {
  std::ofstream out(path);
  if (!out.is_open()) {
    std::cerr << "FAILED: cannot write " << path << "\n";
    std::exit(1);
  }
  for (const std::string& line : lines) {
    out << line << "\n";
  }
}

void test_ascii_art_default_and_line_color_tags_with_indentation() {
  const std::filesystem::path root =
      std::filesystem::temp_directory_path() / "cli_adventure_renderer_tests";
  std::filesystem::create_directories(root);

  const std::filesystem::path art_path = root / "art.txt";
  write_text_file(art_path, {
                               "[default_color=bright_green]",
                               "  base line",
                               "   [color=bright_red]eyes",
                               "  [color:bright_cyan]visor",
                           });

  adventure::ui::Theme theme;
  theme.use_color = true;
  theme.body_color = "default";
  adventure::ui::Renderer renderer(theme);

  std::ostringstream output;
  renderer.render_scene(output, "Test", {}, root.string(), "./art.txt");
  const std::string rendered = output.str();

  expect(rendered.find("[default_color=bright_green]") == std::string::npos,
         "Default color directive must not render as art content.");
  expect(rendered.find("[color=bright_red]") == std::string::npos,
         "Line color tags must not render as raw tag text.");
  expect(rendered.find("\033[92m  base line\033[0m") != std::string::npos,
         "Untagged line should use default art color.");
  expect(rendered.find("\033[91m   eyes\033[0m") != std::string::npos,
         "Indented line color tag should be applied and indentation preserved.");
  expect(rendered.find("\033[96m  visor\033[0m") != std::string::npos,
         "Colon-form line color tag should be applied.");
}

}  // namespace

int main() {
  test_ascii_art_default_and_line_color_tags_with_indentation();
  return 0;
}
