#include <iostream>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "context/game_context.h"
#include "engine/engine.h"
#include "ui/renderer.h"
#include "ui/terminal_menu.h"
#include "ui/theme.h"
#include "validation/game_validator.h"

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

std::vector<std::filesystem::path> discover_theme_files(const std::filesystem::path& theme_root) {
  std::vector<std::filesystem::path> themes;
  if (!std::filesystem::exists(theme_root) || !std::filesystem::is_directory(theme_root)) {
    return themes;
  }

  for (const auto& entry : std::filesystem::directory_iterator(theme_root)) {
    if (!entry.is_regular_file() || entry.path().extension() != ".theme") {
      continue;
    }
    themes.push_back(entry.path().lexically_normal());
  }
  std::sort(themes.begin(), themes.end());
  return themes;
}

adventure::ui::Theme load_runtime_theme(const std::filesystem::path& theme_file) {
  adventure::ui::Theme theme;
  theme = adventure::ui::load_theme_from_file(theme_file, theme);
  theme.use_color = theme.use_color && adventure::ui::is_color_enabled_for_terminal();
  return theme;
}

void run_session(const std::filesystem::path& game_root, const adventure::ui::Theme& theme) {
  const std::filesystem::path entry_level = (game_root / "start.level").lexically_normal();
  std::cout << "\nLaunching: " << game_root.filename().string() << "\n";

  adventure::context::GameContext context;
  context.set_current_directory(game_root.string());
  context.set_current_level_path(entry_level.string());

  adventure::engine::Engine engine{adventure::ui::Renderer(theme)};
  engine.run(std::cin, std::cout, context);
}

void run_validation(const std::filesystem::path& game_root, const adventure::ui::Theme& theme) {
  const adventure::validation::ValidationReport report =
      adventure::validation::validate_game(game_root);

  std::vector<std::string> lines;
  lines.push_back("Checked files: " + std::to_string(report.checked_files));
  if (report.issues.empty()) {
    lines.push_back("No issues found.");
  } else {
    lines.push_back("Issues found: " + std::to_string(report.issues.size()));
    for (const auto& issue : report.issues) {
      lines.push_back("- " + issue.file.string() + ": " + issue.message);
    }
  }

  adventure::ui::Renderer renderer(theme);
  renderer.render_scene(std::cout, "Validation: " + game_root.filename().string(), lines, "", "");
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

  if (discover_games(options.games_root).empty()) {
    std::cerr << "No playable games found in: " << options.games_root.string()
              << " (each game must contain start.level)\n";
    return 1;
  }

  while (true) {
    adventure::ui::Theme theme = load_runtime_theme(options.theme_file);
    const std::vector<std::string> main_options = {"Play Game", "Settings", "Validate Games", "Exit"};

    const adventure::ui::MenuSelection main_selection = adventure::ui::pick_option(
        std::cin, std::cout, main_options, "Main Menu", theme);
    adventure::ui::clear_menu_block(std::cin, std::cout, main_selection.rendered_lines);

    if (main_selection.index == 0) {
      const std::vector<std::filesystem::path> games = discover_games(options.games_root);
      std::vector<std::string> game_names;
      game_names.reserve(games.size());
      for (const auto& game_path : games) {
        game_names.push_back(game_path.filename().string());
      }
      game_names.push_back("Back");

      const adventure::ui::MenuSelection game_selection = adventure::ui::pick_option(
          std::cin, std::cout, game_names, "Choose a game:", theme);
      adventure::ui::clear_menu_block(std::cin, std::cout, game_selection.rendered_lines);
      if (game_selection.index == games.size()) {
        continue;
      }
      run_session(games[game_selection.index], theme);
      continue;
    }

    if (main_selection.index == 1) {
      const std::vector<std::filesystem::path> theme_files =
          discover_theme_files(options.theme_file.parent_path().empty()
                                   ? std::filesystem::path("themes")
                                   : options.theme_file.parent_path());
      if (theme_files.empty()) {
        adventure::ui::wait_for_continue(
            std::cin, std::cout, "\nNo .theme files found. Press Enter to continue...");
        continue;
      }

      std::vector<std::string> theme_names;
      theme_names.reserve(theme_files.size());
      for (const auto& path : theme_files) {
        theme_names.push_back(path.filename().string());
      }
      theme_names.push_back("Back");
      const adventure::ui::MenuSelection theme_selection = adventure::ui::pick_option(
          std::cin, std::cout, theme_names, "Choose theme:", theme);
      adventure::ui::clear_menu_block(std::cin, std::cout, theme_selection.rendered_lines);
      if (theme_selection.index == theme_files.size()) {
        continue;
      }
      options.theme_file = theme_files[theme_selection.index];
      continue;
    }

    if (main_selection.index == 2) {
      const std::vector<std::filesystem::path> games = discover_games(options.games_root);
      std::vector<std::string> game_names;
      game_names.reserve(games.size());
      for (const auto& game_path : games) {
        game_names.push_back(game_path.filename().string());
      }
      game_names.push_back("Back");
      const adventure::ui::MenuSelection game_selection = adventure::ui::pick_option(
          std::cin, std::cout, game_names, "Validate which game?", theme);
      adventure::ui::clear_menu_block(std::cin, std::cout, game_selection.rendered_lines);
      if (game_selection.index == games.size()) {
        continue;
      }
      run_validation(games[game_selection.index], theme);
      continue;
    }

    return 0;
  }
}
