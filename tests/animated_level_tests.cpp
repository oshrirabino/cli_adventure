#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "context/game_context.h"
#include "levels/animated_level_decorator.h"
#include "levels/ilevel.h"
#include "levels/terminal_level_factory.h"
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

void write_text_file(const std::filesystem::path& path, const std::string& content) {
  std::ofstream out(path);
  if (!out.is_open()) {
    std::cerr << "FAILED: cannot write " << path << "\n";
    std::exit(1);
  }
  out << content;
}

class StubLevel final : public adventure::levels::ILevel {
 public:
  void render(std::ostream& out, const adventure::context::GameContext& context) const override {
    (void)out;
    (void)context;
  }

  void execute(std::istream& in, std::ostream& out,
               adventure::context::GameContext& context) override {
    (void)in;
    (void)out;
    context.request_next_level("./next.level");
  }
};

void test_factory_wraps_level_when_ascii_mode_is_animated() {
  adventure::ui::Theme theme;
  adventure::ui::Renderer renderer(theme);
  adventure::levels::TerminalLevelFactory factory(renderer);

  adventure::parser::ParsedLevelData data;
  data.header["title"] = "Animated";
  data.header["ascii_art"] = "./art.txt";
  data.content_lines = {"Line"};
  data.options.push_back({"", "Go", "./next.level"});
  data.directives["input_mode"] = "choice";
  data.directives["ascii_art_mode"] = "animated";

  std::unique_ptr<adventure::levels::ILevel> level = factory.create(data);
  expect(dynamic_cast<adventure::levels::AnimatedLevelDecorator*>(level.get()) != nullptr,
         "Factory should wrap animated levels with AnimatedLevelDecorator.");
}

void test_decorator_renders_last_frame_in_non_tty_and_delegates_execute() {
  const std::filesystem::path root =
      std::filesystem::temp_directory_path() / "cli_adventure_animated_level_tests";
  std::filesystem::create_directories(root);
  write_text_file(root / "anim.txt", R"([fps=20]
[frame]
frame_one
[frame]
frame_two
)");

  adventure::parser::ParsedLevelData data;
  data.header["title"] = "Anim";
  data.header["ascii_art"] = "./anim.txt";
  data.content_lines = {"Room body"};
  data.directives["input_mode"] = "choice";
  data.directives["ascii_art_mode"] = "animated";

  adventure::ui::Theme theme;
  theme.use_color = false;
  adventure::ui::Renderer renderer(theme);
  auto wrapped = std::make_unique<StubLevel>();
  adventure::levels::AnimatedLevelDecorator decorator(std::move(wrapped), data, renderer);

  adventure::context::GameContext context;
  context.set_current_directory(root.string());

  std::ostringstream rendered;
  decorator.render(rendered, context);
  const std::string output = rendered.str();
  expect(output.find("frame_two") != std::string::npos,
         "Decorator should render final frame in non-interactive output.");
  expect(output.find("frame_one") == std::string::npos,
         "Decorator should not animate intermediate frames in non-interactive output.");

  std::istringstream input_stream("");
  std::ostringstream exec_out;
  decorator.execute(input_stream, exec_out, context);
  expect(context.has_next_level_request(), "Decorator should delegate execute to wrapped level.");
  expect(context.next_level_request() == "./next.level",
         "Decorator execute delegation should preserve next level request.");
}

void test_decorator_applies_animated_color_tags() {
  const std::filesystem::path root =
      std::filesystem::temp_directory_path() / "cli_adventure_animated_level_color_tests";
  std::filesystem::create_directories(root);
  write_text_file(root / "anim_color.txt", R"([fps=3]
[frame]
[color=bright_blue]old
[frame]
[color=bright_red]final
)");

  adventure::parser::ParsedLevelData data;
  data.header["title"] = "Anim Color";
  data.header["ascii_art"] = "./anim_color.txt";
  data.content_lines = {"Room body"};
  data.directives["input_mode"] = "choice";
  data.directives["ascii_art_mode"] = "animated";

  adventure::ui::Theme theme;
  theme.use_color = true;
  adventure::ui::Renderer renderer(theme);
  auto wrapped = std::make_unique<StubLevel>();
  adventure::levels::AnimatedLevelDecorator decorator(std::move(wrapped), data, renderer);

  adventure::context::GameContext context;
  context.set_current_directory(root.string());

  std::ostringstream rendered;
  decorator.render(rendered, context);
  const std::string output = rendered.str();
  expect(output.find("[color=bright_red]") == std::string::npos,
         "Animated color tags must not render as raw text.");
  expect(output.find("\033[91mfinal\033[0m") != std::string::npos,
         "Animated color tag should apply ANSI color to frame content.");
}

}  // namespace

int main() {
  test_factory_wraps_level_when_ascii_mode_is_animated();
  test_decorator_renders_last_frame_in_non_tty_and_delegates_execute();
  test_decorator_applies_animated_color_tags();
  return 0;
}
