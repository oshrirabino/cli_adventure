#include "ui/renderer.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>
#include <unistd.h>

namespace adventure::ui {

Renderer::Renderer(Theme theme) : theme_(std::move(theme)) {}

const Theme& Renderer::theme() const { return theme_; }

void Renderer::render_scene(std::ostream& out, const std::string& title,
                            const std::vector<std::string>& content_lines,
                            const std::string& current_directory,
                            const std::string& ascii_art_relative_path) const {
  std::size_t rendered_lines = 0;

  out << "\n" << colorize(theme_.border_line, theme_.title_color) << "\n";
  out << colorize(title, theme_.title_color) << "\n";
  out << colorize(theme_.border_line, theme_.title_color) << "\n\n";
  rendered_lines += 5;

  for (const std::string& line : content_lines) {
    out << colorize(line, theme_.body_color) << "\n";
  }
  rendered_lines += content_lines.size();

  if (!ascii_art_relative_path.empty()) {
    const std::vector<std::string> art =
        load_ascii_art(current_directory, ascii_art_relative_path);
    if (art.empty()) {
      render_structure_error(out, "ASCII art not found: " + ascii_art_relative_path);
      rendered_lines += 2;
    } else {
      for (const std::string& line : art) {
        out << colorize(line, theme_.body_color) << "\n";
      }
      out << "\n";
      rendered_lines += art.size() + 1;
    }
  }

  last_scene_lines_ = rendered_lines;
}

void Renderer::clear_last_scene(std::ostream& out, std::size_t extra_lines_after_scene) const {
  if (&out != &std::cout || !isatty(STDOUT_FILENO)) {
    return;
  }

  const std::size_t total_lines = last_scene_lines_ + extra_lines_after_scene;
  if (total_lines == 0) {
    return;
  }

  out << "\033[" << total_lines << "A";
  for (std::size_t i = 0; i < total_lines; ++i) {
    out << "\033[2K\r";
    if (i + 1 < total_lines) {
      out << "\033[1B";
    }
  }
  out << "\033[" << (total_lines - 1) << "A";
  out.flush();
}

void Renderer::render_victory(std::ostream& out) const {
  out << "\n" << colorize("[Victory]", theme_.victory_color) << "\n";
}

void Renderer::render_game_over(std::ostream& out) const {
  out << "\n" << colorize("[Game Over]", theme_.game_over_color) << "\n";
}

void Renderer::render_structure_error(std::ostream& out, const std::string& message) const {
  out << "\n" << colorize("[Structure Error] " + message, theme_.error_color) << "\n";
}

std::string Renderer::colorize(const std::string& text, const std::string& color_name) const {
  if (!theme_.use_color) {
    return text;
  }

  const std::string code = ansi_color_code(color_name);
  if (code.empty()) {
    return text;
  }
  return code + text + "\033[0m";
}

std::vector<std::string> Renderer::load_ascii_art(
    const std::string& current_directory, const std::string& ascii_art_relative_path) const {
  std::filesystem::path full_path =
      std::filesystem::path(current_directory) / ascii_art_relative_path;
  full_path = full_path.lexically_normal();

  std::ifstream in(full_path);
  if (!in.is_open()) {
    return {};
  }

  std::vector<std::string> lines;
  std::string line;
  while (std::getline(in, line)) {
    lines.push_back(line);
  }
  return lines;
}

}  // namespace adventure::ui
