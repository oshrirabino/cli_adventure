#ifndef CLI_ADVENTURE_UI_TERMINAL_MENU_H_
#define CLI_ADVENTURE_UI_TERMINAL_MENU_H_

#include <cstddef>
#include <functional>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

#include "ui/theme.h"

namespace adventure::ui {

bool supports_interactive_menu(const std::istream& in, const std::ostream& out);

struct MenuSelection {
  std::size_t index = 0;
  std::size_t rendered_lines = 0;
};

struct MenuRenderHooks {
  std::size_t extra_lines = 0;
  std::size_t tick_interval_ms = 0;
  std::function<void(std::ostream& out)> render_extra_block;
  std::function<void(std::ostream& out, std::size_t menu_lines)> render_extra_in_place;
  std::function<void()> on_idle_tick;
};

MenuSelection pick_option(std::istream& in, std::ostream& out,
                          const std::vector<std::string>& options, const std::string& prompt,
                          const Theme& theme, const MenuRenderHooks* hooks = nullptr);
void clear_menu_block(std::istream& in, std::ostream& out, std::size_t rendered_lines);
void wait_for_continue(std::istream& in, std::ostream& out, const std::string& prompt);

}  // namespace adventure::ui

#endif  // CLI_ADVENTURE_UI_TERMINAL_MENU_H_
