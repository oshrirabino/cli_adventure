#include "levels/animated_level_decorator.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <utility>

#include <unistd.h>

#include "ui/terminal_menu.h"
#include "ui/theme.h"

namespace adventure::levels {
namespace {

std::string read_header_value(const std::unordered_map<std::string, std::string>& map,
                              const std::string& key) {
  const auto it = map.find(key);
  if (it == map.end()) {
    return "";
  }
  return it->second;
}

struct ColoredLine {
  std::string text;
  std::string color_name;
};

std::string trim_local(std::string value) {
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

std::string to_lower_local(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return value;
}

ColoredLine parse_colored_line(const std::string& raw_line) {
  ColoredLine line{raw_line, ""};
  std::size_t tag_start = 0;
  while (tag_start < raw_line.size() &&
         std::isspace(static_cast<unsigned char>(raw_line[tag_start])) != 0) {
    ++tag_start;
  }
  if (tag_start >= raw_line.size() || raw_line[tag_start] != '[') {
    return line;
  }

  const std::size_t tag_end = raw_line.find(']', tag_start + 1);
  if (tag_end == std::string::npos) {
    return line;
  }

  const std::string tag = to_lower_local(trim_local(
      raw_line.substr(tag_start + 1, tag_end - tag_start - 1)));
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
    return line;
  }
  const std::string key = trim_local(tag.substr(0, delimiter));
  const std::string value = trim_local(tag.substr(delimiter + 1));
  if (key != "color" || value.empty()) {
    return line;
  }

  line.text = raw_line.substr(0, tag_start) + raw_line.substr(tag_end + 1);
  line.color_name = value;
  return line;
}

void clear_block(std::ostream& out, std::size_t lines) {
  if (lines == 0) {
    return;
  }
  out << "\033[" << lines << "A";
  for (std::size_t i = 0; i < lines; ++i) {
    out << "\033[2K\r";
    if (i + 1 < lines) {
      out << "\033[1B";
    }
  }
  out << "\033[" << (lines - 1) << "A";
}

}  // namespace

AnimatedLevelDecorator::AnimatedLevelDecorator(std::unique_ptr<ILevel> wrapped,
                                               adventure::parser::ParsedLevelData data,
                                               const adventure::ui::Renderer& renderer)
    : wrapped_(std::move(wrapped)),
      title_(build_title(data)),
      ascii_art_path_(read_header_value(data.header, "ascii_art")),
      content_lines_(std::move(data.content_lines)),
      renderer_(renderer) {}

void AnimatedLevelDecorator::render(std::ostream& out,
                                    const adventure::context::GameContext& context) const {
  if (ascii_art_path_.empty()) {
    wrapped_->render(out, context);
    return;
  }

  const std::filesystem::path full_path =
      (std::filesystem::path(context.current_directory()) / ascii_art_path_).lexically_normal();

  AnimationData animation;
  if (!parse_animation_file(full_path, &animation) || animation.frames.empty()) {
    renderer_.render_scene(out, title_, content_lines_, context.current_directory(), ascii_art_path_);
    return;
  }

  const adventure::ui::Theme& theme = renderer_.theme();
  std::size_t max_frame_lines = 0;
  for (const auto& frame : animation.frames) {
    max_frame_lines = std::max(max_frame_lines, frame.size());
  }

  const bool is_interactive_output = (&out == &std::cout) && isatty(STDOUT_FILENO);
  const std::vector<std::string>& initial_frame =
      is_interactive_output ? animation.frames.front() : animation.frames.back();
  render_scene_with_frame(out, theme, title_, content_lines_, initial_frame, max_frame_lines);
  renderer_.record_last_scene_lines(5 + content_lines_.size() + max_frame_lines + 1);
}

void AnimatedLevelDecorator::execute(std::istream& in, std::ostream& out,
                                     adventure::context::GameContext& context) {
  const bool is_interactive = adventure::ui::supports_interactive_menu(in, out);
  if (!is_interactive || ascii_art_path_.empty()) {
    wrapped_->execute(in, out, context);
    return;
  }

  const std::filesystem::path full_path =
      (std::filesystem::path(context.current_directory()) / ascii_art_path_).lexically_normal();
  AnimationData animation;
  if (!parse_animation_file(full_path, &animation) || animation.frames.size() <= 1) {
    wrapped_->execute(in, out, context);
    return;
  }

  std::size_t max_frame_lines = 0;
  for (const auto& frame : animation.frames) {
    max_frame_lines = std::max(max_frame_lines, frame.size());
  }

  const std::size_t scene_lines = 5 + content_lines_.size() + max_frame_lines + 1;
  const std::size_t tick_interval_ms =
      static_cast<std::size_t>(1000 / std::max(1, std::min(animation.fps, 120)));
  std::size_t frame_index = 0;

  const adventure::ui::Theme& theme = renderer_.theme();
  adventure::ui::MenuRenderHooks hooks;
  hooks.extra_lines = scene_lines;
  hooks.tick_interval_ms = tick_interval_ms;
  hooks.on_idle_tick = [&]() { frame_index = (frame_index + 1) % animation.frames.size(); };
  hooks.render_extra_block = [&](std::ostream& menu_out) {
    render_scene_with_frame(menu_out, theme, title_, content_lines_, animation.frames[frame_index],
                            max_frame_lines);
  };
  hooks.render_extra_in_place = [&](std::ostream& menu_out, std::size_t menu_lines) {
    const std::size_t art_block_lines = max_frame_lines + 1;
    menu_out << "\033[" << menu_lines << "A";
    clear_block(menu_out, art_block_lines);
    for (const std::string& raw_line : animation.frames[frame_index]) {
      const ColoredLine parsed = parse_colored_line(raw_line);
      const std::string color = parsed.color_name.empty() ? theme.body_color : parsed.color_name;
      menu_out << colorize(parsed.text, theme, color) << "\n";
    }
    for (std::size_t i = animation.frames[frame_index].size(); i < max_frame_lines; ++i) {
      menu_out << "\n";
    }
    menu_out << "\n";
    menu_out << "\033[" << menu_lines << "B";
  };

  struct ScopedHooks {
    const adventure::ui::Renderer& renderer;
    explicit ScopedHooks(const adventure::ui::Renderer& r,
                         const adventure::ui::MenuRenderHooks* active_hooks)
        : renderer(r) {
      renderer.set_menu_render_hooks(active_hooks);
    }
    ~ScopedHooks() { renderer.set_menu_render_hooks(nullptr); }
  } scoped_hooks(renderer_, &hooks);

  wrapped_->execute(in, out, context);
}

std::string AnimatedLevelDecorator::build_title(const adventure::parser::ParsedLevelData& data) {
  const auto title_it = data.header.find("title");
  if (title_it != data.header.end() && !title_it->second.empty()) {
    return title_it->second;
  }

  const auto id_it = data.header.find("id");
  if (id_it != data.header.end() && !id_it->second.empty()) {
    return "Level: " + id_it->second;
  }

  return "Untitled Level";
}

bool AnimatedLevelDecorator::parse_animation_file(const std::filesystem::path& path,
                                                  AnimationData* animation) {
  std::ifstream in(path);
  if (!in.is_open()) {
    return false;
  }

  animation->fps = 12;
  animation->frames.clear();

  std::vector<std::string> current_frame;
  bool seen_frame_separator = false;
  std::string raw_line;
  while (std::getline(in, raw_line)) {
    const std::string normalized = to_lower(trim(raw_line));
    if (normalized.empty()) {
      current_frame.push_back(raw_line);
      continue;
    }

    if (normalized == "[frame]") {
      seen_frame_separator = true;
      if (!current_frame.empty()) {
        animation->frames.push_back(std::move(current_frame));
        current_frame = {};
      }
      continue;
    }

    if (normalized.rfind("[fps=", 0) == 0 && normalized.back() == ']') {
      const std::string fps_value = trim(normalized.substr(5, normalized.size() - 6));
      if (!fps_value.empty()) {
        try {
          animation->fps = std::stoi(fps_value);
        } catch (...) {
          animation->fps = 12;
        }
      }
      continue;
    }

    current_frame.push_back(raw_line);
  }

  if (!current_frame.empty()) {
    animation->frames.push_back(std::move(current_frame));
  }

  if (!seen_frame_separator) {
    return !animation->frames.empty();
  }

  std::vector<std::vector<std::string>> non_empty_frames;
  for (auto& frame : animation->frames) {
    if (!frame.empty()) {
      non_empty_frames.push_back(std::move(frame));
    }
  }
  animation->frames = std::move(non_empty_frames);
  animation->fps = std::max(1, std::min(animation->fps, 120));
  return !animation->frames.empty();
}

std::string AnimatedLevelDecorator::trim(std::string value) {
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

std::string AnimatedLevelDecorator::to_lower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return value;
}

std::string AnimatedLevelDecorator::colorize(const std::string& text,
                                             const adventure::ui::Theme& theme,
                                             const std::string& color_name) {
  if (!theme.use_color) {
    return text;
  }

  const std::string code = adventure::ui::ansi_color_code(color_name);
  if (code.empty()) {
    return text;
  }
  return code + text + "\033[0m";
}

void AnimatedLevelDecorator::render_scene_with_frame(
    std::ostream& out, const adventure::ui::Theme& theme, const std::string& title,
    const std::vector<std::string>& content_lines, const std::vector<std::string>& frame_lines,
    std::size_t max_frame_lines) {
  out << "\n" << colorize(theme.border_line, theme, theme.title_color) << "\n";
  out << colorize(title, theme, theme.title_color) << "\n";
  out << colorize(theme.border_line, theme, theme.title_color) << "\n\n";
  for (const std::string& line : content_lines) {
    out << colorize(line, theme, theme.body_color) << "\n";
  }
  for (const std::string& raw_line : frame_lines) {
    const ColoredLine parsed = parse_colored_line(raw_line);
    const std::string color = parsed.color_name.empty() ? theme.body_color : parsed.color_name;
    out << colorize(parsed.text, theme, color) << "\n";
  }
  for (std::size_t i = frame_lines.size(); i < max_frame_lines; ++i) {
    out << "\n";
  }
  out << "\n";
}

}  // namespace adventure::levels
