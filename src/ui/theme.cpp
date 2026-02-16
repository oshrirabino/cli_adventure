#include "ui/theme.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <string>
#include <unordered_map>

#include <unistd.h>

namespace adventure::ui {
namespace {

std::string trim(std::string value) {
  const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
    return std::isspace(ch) != 0;
  });
  if (first == value.end()) {
    return "";
  }
  const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
    return std::isspace(ch) != 0;
  }).base();
  return std::string(first, last);
}

std::string to_lower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return value;
}

bool parse_bool(const std::string& value, bool* result) {
  const std::string lowered = to_lower(trim(value));
  if (lowered == "true" || lowered == "1" || lowered == "yes" || lowered == "on") {
    *result = true;
    return true;
  }
  if (lowered == "false" || lowered == "0" || lowered == "no" || lowered == "off") {
    *result = false;
    return true;
  }
  return false;
}

}  // namespace

Theme load_theme_from_file(const std::filesystem::path& theme_file, const Theme& base_theme) {
  Theme theme = base_theme;
  std::ifstream in(theme_file);
  if (!in.is_open()) {
    return theme;
  }

  std::string line;
  while (std::getline(in, line)) {
    const std::string trimmed = trim(line);
    if (trimmed.empty() || trimmed[0] == '#') {
      continue;
    }

    const auto eq = trimmed.find('=');
    if (eq == std::string::npos) {
      continue;
    }

    const std::string key = to_lower(trim(trimmed.substr(0, eq)));
    const std::string value = trim(trimmed.substr(eq + 1));
    if (key.empty()) {
      continue;
    }

    if (key == "border_line") {
      theme.border_line = value;
    } else if (key == "menu_selected_prefix") {
      theme.menu_selected_prefix = value;
    } else if (key == "menu_unselected_prefix") {
      theme.menu_unselected_prefix = value;
    } else if (key == "title_color") {
      theme.title_color = value;
    } else if (key == "body_color") {
      theme.body_color = value;
    } else if (key == "prompt_color") {
      theme.prompt_color = value;
    } else if (key == "selected_color") {
      theme.selected_color = value;
    } else if (key == "unselected_color") {
      theme.unselected_color = value;
    } else if (key == "victory_color") {
      theme.victory_color = value;
    } else if (key == "game_over_color") {
      theme.game_over_color = value;
    } else if (key == "error_color") {
      theme.error_color = value;
    } else if (key == "use_color") {
      bool parsed = theme.use_color;
      if (parse_bool(value, &parsed)) {
        theme.use_color = parsed;
      }
    }
  }

  return theme;
}

bool is_color_enabled_for_terminal() {
  if (!isatty(STDOUT_FILENO)) {
    return false;
  }
  const char* no_color = std::getenv("NO_COLOR");
  return no_color == nullptr;
}

std::string ansi_color_code(const std::string& color_name) {
  static const std::unordered_map<std::string, std::string> kCodes = {
      {"default", "\033[39m"},
      {"black", "\033[30m"},
      {"red", "\033[31m"},
      {"green", "\033[32m"},
      {"yellow", "\033[33m"},
      {"blue", "\033[34m"},
      {"magenta", "\033[35m"},
      {"cyan", "\033[36m"},
      {"white", "\033[37m"},
      {"bright_black", "\033[90m"},
      {"bright_red", "\033[91m"},
      {"bright_green", "\033[92m"},
      {"bright_yellow", "\033[93m"},
      {"bright_blue", "\033[94m"},
      {"bright_magenta", "\033[95m"},
      {"bright_cyan", "\033[96m"},
      {"bright_white", "\033[97m"},
      {"none", ""},
      {"", ""},
  };

  const auto it = kCodes.find(to_lower(trim(color_name)));
  if (it == kCodes.end()) {
    return "";
  }
  return it->second;
}

}  // namespace adventure::ui
