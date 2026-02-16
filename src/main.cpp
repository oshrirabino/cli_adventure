#include <iostream>
#include <algorithm>
#include <filesystem>
#include <exception>
#include <string>
#include <vector>

#include "context/game_context.h"
#include "engine/engine.h"
#include "ui/renderer.h"
#include "ui/terminal_menu.h"
#include "ui/theme.h"

namespace {

struct CliOptions {
  std::filesystem::path games_root = "games";
  std::filesystem::path theme_file = "themes/default.theme";
};

int print_usage(const char* program_name) {
  std::cerr << "Usage: " << program_name << " [games_directory] [--theme <theme_file>]\n";
  std::cerr << "Default games directory: ./games\n";
  std::cerr << "Default theme file: ./themes/default.theme\n";
  std::cerr << "Example: " << program_name << "\n";
  std::cerr << "Example: " << program_name << " ./games\n";
  std::cerr << "Example: " << program_name << " ./games --theme ./themes/default.theme\n";
  return 1;
}

bool parse_args(int argc, char** argv, CliOptions* options) {
  if (argc == 1) {
    return true;
  }
  if (argc == 2) {
    if (std::string(argv[1]) == "--theme") {
      return false;
    }
    options->games_root = std::filesystem::path(argv[1]).lexically_normal();
    return true;
  }
  if (argc == 3) {
    if (std::string(argv[1]) != "--theme") {
      return false;
    }
    options->theme_file = std::filesystem::path(argv[2]).lexically_normal();
    return true;
  }
  if (argc == 4) {
    if (std::string(argv[2]) != "--theme") {
      return false;
    }
    options->games_root = std::filesystem::path(argv[1]).lexically_normal();
    options->theme_file = std::filesystem::path(argv[3]).lexically_normal();
    return true;
  }
  return false;
}

std::vector<std::filesystem::path> discover_games(const std::filesystem::path& games_root) {
  std::vector<std::filesystem::path> games;
  for (const auto& entry : std::filesystem::directory_iterator(games_root)) {
    if (!entry.is_directory()) {
      continue;
    }
    const std::filesystem::path candidate_root = entry.path();
    const std::filesystem::path start_file = candidate_root / "start.level";
    if (std::filesystem::exists(start_file) && std::filesystem::is_regular_file(start_file)) {
      games.push_back(candidate_root.lexically_normal());
    }
  }

  std::sort(games.begin(), games.end(),
            [](const std::filesystem::path& left, const std::filesystem::path& right) {
              return left.filename().string() < right.filename().string();
            });
  return games;
}

}  // namespace

int main(int argc, char** argv) {
  CliOptions options;
  if (!parse_args(argc, argv, &options)) {
    return print_usage(argv[0]);
  }

  if (!std::filesystem::exists(options.games_root) ||
      !std::filesystem::is_directory(options.games_root)) {
    std::cerr << "Games directory does not exist or is not a directory: "
              << options.games_root.string() << "\n";
    return 1;
  }

  const std::vector<std::filesystem::path> games = discover_games(options.games_root);
  if (games.empty()) {
    std::cerr << "No playable games found in: " << options.games_root.string()
              << " (each game must contain start.level)\n";
    return 1;
  }

  std::vector<std::string> game_names;
  game_names.reserve(games.size());
  for (const auto& game_path : games) {
    game_names.push_back(game_path.filename().string());
  }

  adventure::ui::Theme theme;
  theme = adventure::ui::load_theme_from_file(options.theme_file, theme);
  theme.use_color = theme.use_color && adventure::ui::is_color_enabled_for_terminal();
  adventure::ui::Renderer renderer(theme);

  try {
    const adventure::ui::MenuSelection selection =
        adventure::ui::pick_option(std::cin, std::cout, game_names, "Select a game:", theme);
    const std::filesystem::path game_root = games[selection.index];
    const std::filesystem::path entry_level = (game_root / "start.level").lexically_normal();

    std::cout << "\nLaunching: " << game_root.filename().string() << "\n";

    adventure::context::GameContext context;
    context.set_current_directory(game_root.string());
    context.set_current_level_path(entry_level.string());

    adventure::engine::Engine engine(renderer);
    engine.run(std::cin, std::cout, context);
  } catch (const std::exception& ex) {
    std::cerr << "Runtime error: " << ex.what() << "\n";
    return 1;
  }

  return 0;
}
