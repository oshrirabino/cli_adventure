#include "ui/renderer.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>
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

struct ParsedLeadingTag {
  bool ok = false;
  std::size_t tag_start = 0;
  std::size_t tag_end = 0;
  std::string key;
  std::string value;
};

ParsedLeadingTag parse_leading_tag(const std::string& raw_line) {
  ParsedLeadingTag parsed;
  std::size_t tag_start = 0;
  while (tag_start < raw_line.size() &&
         std::isspace(static_cast<unsigned char>(raw_line[tag_start])) != 0) {
    ++tag_start;
  }
  if (tag_start >= raw_line.size() || raw_line[tag_start] != '[') {
    return parsed;
  }

  const std::size_t tag_end = raw_line.find(']', tag_start + 1);
  if (tag_end == std::string::npos) {
    return parsed;
  }

  const std::string tag = trim(raw_line.substr(tag_start + 1, tag_end - tag_start - 1));
  if (tag.empty()) {
    return parsed;
  }

  const std::size_t eq = tag.find('=');
  const std::size_t colon = tag.find(':');
  std::size_t delimiter = std::string::npos;
  if (eq != std::string::npos && colon != std::string::npos) {
    delimiter = std::min(eq, colon);
  } else if (eq != std::string::npos) {
    delimiter = eq;
  } else if (colon != std::string::npos) {
    delimiter = colon;
  }
  if (delimiter == std::string::npos) {
    return parsed;
  }

  const std::string key = to_lower(trim(tag.substr(0, delimiter)));
  const std::string value = trim(tag.substr(delimiter + 1));
  if (key.empty() || value.empty()) {
    return parsed;
  }

  parsed.ok = true;
  parsed.tag_start = tag_start;
  parsed.tag_end = tag_end;
  parsed.key = key;
  parsed.value = value;
  return parsed;
}

bool is_whitespace_only(const std::string& text) {
  for (char ch : text) {
    if (std::isspace(static_cast<unsigned char>(ch)) == 0) {
      return false;
    }
  }
  return true;
}

bool parse_default_color_directive(const std::string& raw_line, std::string* color_name) {
  const ParsedLeadingTag parsed = parse_leading_tag(raw_line);
  if (!parsed.ok) {
    return false;
  }
  if (parsed.key != "default_color" && parsed.key != "art_color") {
    return false;
  }

  const std::string trailing = raw_line.substr(parsed.tag_end + 1);
  if (!is_whitespace_only(trailing)) {
    return false;
  }

  *color_name = parsed.value;
  return true;
}

}  // namespace

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
    const std::vector<AsciiArtLine> art =
        load_ascii_art(current_directory, ascii_art_relative_path);
    if (art.empty()) {
      render_structure_error(out, "ASCII art not found: " + ascii_art_relative_path);
      rendered_lines += 2;
    } else {
      for (const AsciiArtLine& line : art) {
        const std::string color_name =
            line.color_name.empty() ? theme_.body_color : line.color_name;
        out << colorize(line.text, color_name) << "\n";
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

Renderer::AsciiArtLine Renderer::parse_ascii_art_line(const std::string& raw_line) {
  AsciiArtLine parsed{raw_line, ""};
  const ParsedLeadingTag tag = parse_leading_tag(raw_line);
  if (!tag.ok || tag.key != "color") {
    return parsed;
  }

  parsed.text = raw_line.substr(0, tag.tag_start) + raw_line.substr(tag.tag_end + 1);
  parsed.color_name = tag.value;
  return parsed;
}

bool Renderer::parse_ascii_art_default_color_directive(const std::string& raw_line,
                                                       std::string* color_name) {
  return parse_default_color_directive(raw_line, color_name);
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

std::vector<Renderer::AsciiArtLine> Renderer::load_ascii_art(
    const std::string& current_directory, const std::string& ascii_art_relative_path) const {
  std::filesystem::path full_path =
      std::filesystem::path(current_directory) / ascii_art_relative_path;
  full_path = full_path.lexically_normal();

  std::ifstream in(full_path);
  if (!in.is_open()) {
    return {};
  }

  std::vector<AsciiArtLine> lines;
  std::string default_art_color;
  std::string line;
  while (std::getline(in, line)) {
    if (Renderer::parse_ascii_art_default_color_directive(line, &default_art_color)) {
      continue;
    }

    AsciiArtLine parsed = Renderer::parse_ascii_art_line(line);
    if (parsed.color_name.empty()) {
      parsed.color_name = default_art_color;
    }
    lines.push_back(std::move(parsed));
  }
  return lines;
}

}  // namespace adventure::ui
