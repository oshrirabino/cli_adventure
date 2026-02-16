#include "ui/terminal_menu.h"

#include <algorithm>
#include <cctype>
#include <csignal>
#include <iostream>
#include <stdexcept>
#include <string>

#include <termios.h>
#include <unistd.h>

namespace adventure::ui {
namespace {

enum class Key {
  kUp,
  kDown,
  kEnter,
  kUnknown,
  kEof,
};

termios g_original_termios{};
bool g_has_original_termios = false;
bool g_raw_mode_active = false;
using SignalHandler = void (*)(int);
SignalHandler g_prev_sigint = SIG_DFL;
SignalHandler g_prev_sigterm = SIG_DFL;
SignalHandler g_prev_sighup = SIG_DFL;
SignalHandler g_prev_sigquit = SIG_DFL;

void restore_terminal_state() {
  if (g_raw_mode_active && g_has_original_termios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_original_termios);
    g_raw_mode_active = false;
  }
}

void raw_mode_signal_handler(int signal_number) {
  restore_terminal_state();
  std::signal(signal_number, SIG_DFL);
  raise(signal_number);
}

void install_raw_mode_signal_handlers() {
  g_prev_sigint = std::signal(SIGINT, raw_mode_signal_handler);
  g_prev_sigterm = std::signal(SIGTERM, raw_mode_signal_handler);
  g_prev_sighup = std::signal(SIGHUP, raw_mode_signal_handler);
  g_prev_sigquit = std::signal(SIGQUIT, raw_mode_signal_handler);
}

void uninstall_raw_mode_signal_handlers() {
  std::signal(SIGINT, g_prev_sigint);
  std::signal(SIGTERM, g_prev_sigterm);
  std::signal(SIGHUP, g_prev_sighup);
  std::signal(SIGQUIT, g_prev_sigquit);
}

class ScopedRawMode {
 public:
  ScopedRawMode() {
    if (!isatty(STDIN_FILENO)) {
      return;
    }
    if (tcgetattr(STDIN_FILENO, &original_) != 0) {
      return;
    }
    g_original_termios = original_;
    g_has_original_termios = true;

    termios raw = original_;
    raw.c_lflag &= static_cast<unsigned int>(~(ICANON | ECHO));
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == 0) {
      active_ = true;
      g_raw_mode_active = true;
      install_raw_mode_signal_handlers();
    }
  }

  ~ScopedRawMode() {
    if (active_) {
      restore_terminal_state();
      uninstall_raw_mode_signal_handlers();
    }
  }

 private:
  bool active_ = false;
  termios original_{};
};

Key read_key() {
  char ch = 0;
  const ssize_t n = read(STDIN_FILENO, &ch, 1);
  if (n <= 0) {
    return Key::kEof;
  }

  if (ch == '\n' || ch == '\r') {
    return Key::kEnter;
  }
  if (ch == '\x1b') {
    char seq[2] = {0, 0};
    if (read(STDIN_FILENO, &seq[0], 1) <= 0 || read(STDIN_FILENO, &seq[1], 1) <= 0) {
      return Key::kUnknown;
    }
    if (seq[0] == '[' && seq[1] == 'A') {
      return Key::kUp;
    }
    if (seq[0] == '[' && seq[1] == 'B') {
      return Key::kDown;
    }
    return Key::kUnknown;
  }
  return Key::kUnknown;
}

void render_menu(std::ostream& out, const std::vector<std::string>& options,
                 const std::string& prompt, std::size_t selected, bool redraw,
                 const Theme& theme) {
  const std::size_t lines = options.size() + 1;
  if (redraw) {
    out << "\033[" << lines << "A";
    for (std::size_t i = 0; i < lines; ++i) {
      out << "\033[2K\r";
      if (i + 1 < lines) {
        out << "\033[1B";
      }
    }
    out << "\033[" << (lines - 1) << "A";
  }

  const auto colorize = [&theme](const std::string& text, const std::string& color) {
    if (!theme.use_color) {
      return text;
    }
    const std::string code = ansi_color_code(color);
    if (code.empty()) {
      return text;
    }
    return code + text + "\033[0m";
  };

  out << colorize(prompt, theme.prompt_color) << "\n";
  for (std::size_t index = 0; index < options.size(); ++index) {
    const bool is_selected = index == selected;
    const std::string prefix =
        is_selected ? (theme.menu_selected_prefix + " ") : (theme.menu_unselected_prefix + " ");
    out << colorize(prefix + options[index], is_selected ? theme.selected_color : theme.unselected_color);
    out << "\n";
  }
  out.flush();
}

bool try_parse_index(const std::string& input, std::size_t option_count, std::size_t* index) {
  std::string trimmed = input;
  trimmed.erase(trimmed.begin(),
                std::find_if(trimmed.begin(), trimmed.end(),
                             [](unsigned char c) { return std::isspace(c) == 0; }));
  while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back())) != 0) {
    trimmed.pop_back();
  }
  if (trimmed.empty()) {
    return false;
  }

  std::size_t parsed = 0;
  for (char c : trimmed) {
    if (!std::isdigit(static_cast<unsigned char>(c))) {
      return false;
    }
    parsed = parsed * 10 + static_cast<std::size_t>(c - '0');
  }

  if (parsed < 1 || parsed > option_count) {
    return false;
  }

  *index = parsed - 1;
  return true;
}

MenuSelection pick_option_fallback(std::istream& in, std::ostream& out,
                                   const std::vector<std::string>& options,
                                   const std::string& prompt) {
  out << prompt << "\n";
  for (std::size_t index = 0; index < options.size(); ++index) {
    out << "  [" << (index + 1) << "] " << options[index] << "\n";
  }
  out << "Choose an option [1-" << options.size() << "]: ";

  std::string line;
  while (std::getline(in, line)) {
    std::size_t selected = 0;
    if (try_parse_index(line, options.size(), &selected)) {
      return MenuSelection{selected, options.size() + 2};
    }
    out << "Invalid selection. Enter a number from 1 to " << options.size() << ": ";
  }

  throw std::runtime_error("Input stream closed before a selection was made.");
}

MenuSelection pick_option_interactive(std::ostream& out, const std::vector<std::string>& options,
                                      const std::string& prompt, const Theme& theme) {
  ScopedRawMode raw_mode;
  std::size_t selected = 0;
  render_menu(out, options, prompt, selected, false, theme);

  while (true) {
    const Key key = read_key();
    if (key == Key::kEof) {
      throw std::runtime_error("Input stream closed before a selection was made.");
    }
    if (key == Key::kEnter) {
      out << "\n";
      return MenuSelection{selected, options.size() + 2};
    }
    if (key == Key::kUp) {
      selected = (selected == 0) ? (options.size() - 1) : (selected - 1);
      render_menu(out, options, prompt, selected, true, theme);
      continue;
    }
    if (key == Key::kDown) {
      selected = (selected + 1) % options.size();
      render_menu(out, options, prompt, selected, true, theme);
      continue;
    }
  }
}

}  // namespace

bool supports_interactive_menu(const std::istream& in, const std::ostream& out) {
  return (&in == &std::cin) && (&out == &std::cout) && isatty(STDIN_FILENO) &&
         isatty(STDOUT_FILENO);
}

MenuSelection pick_option(std::istream& in, std::ostream& out,
                          const std::vector<std::string>& options, const std::string& prompt,
                          const Theme& theme) {
  if (options.empty()) {
    throw std::invalid_argument("pick_option requires at least one option.");
  }

  if (supports_interactive_menu(in, out)) {
    return pick_option_interactive(out, options, prompt, theme);
  }

  return pick_option_fallback(in, out, options, prompt);
}

}  // namespace adventure::ui
