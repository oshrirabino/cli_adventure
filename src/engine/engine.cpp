#include "engine/engine.h"

#include <filesystem>
#include <stdexcept>
#include <utility>

namespace adventure::engine {

Engine::Engine() : Engine(adventure::ui::Renderer(adventure::ui::Theme{})) {}

Engine::Engine(adventure::ui::Renderer renderer)
    : renderer_(std::move(renderer)), factory_(renderer_) {}

void Engine::run(std::istream& in, std::ostream& out, adventure::context::GameContext& context) {
  if (context.current_level_path().empty()) {
    throw std::invalid_argument("GameContext.current_level_path must be set before Engine::run.");
  }

  while (!context.is_game_over() && !context.is_victory()) {
    const std::filesystem::path current_level_path(context.current_level_path());
    context.set_current_directory(current_level_path.parent_path().string());

    try {
      const adventure::parser::ParsedLevelData parsed_data =
          parser_.parse_file(context.current_level_path());
      std::unique_ptr<adventure::levels::ILevel> level = factory_.create(parsed_data);
      level->render(out, context);
      level->execute(in, out, context);
    } catch (const std::exception& ex) {
      renderer_.render_structure_error(out,
                                       "Failed to load/execute level `" +
                                           context.current_level_path() + "`: " + ex.what());
      context.set_game_over(true);
      break;
    }

    if (!context.has_next_level_request() && !context.is_game_over() && !context.is_victory()) {
      renderer_.render_structure_error(
          out, "Level did not request next level or terminate: " + context.current_level_path());
      context.set_game_over(true);
      break;
    }

    if (!context.has_next_level_request()) {
      continue;
    }

    const std::string resolved_path = resolve_next_level_path(
        context.current_level_path(), context.next_level_request());
    context.set_current_level_path(resolved_path);
    context.clear_next_level_request();
  }
}

std::string Engine::resolve_next_level_path(const std::string& current_level_path,
                                            const std::string& relative_or_absolute_next) {
  const std::filesystem::path next_path(relative_or_absolute_next);
  if (next_path.is_absolute()) {
    return next_path.lexically_normal().string();
  }

  const std::filesystem::path current_path(current_level_path);
  const std::filesystem::path combined = current_path.parent_path() / next_path;
  return combined.lexically_normal().string();
}

}  // namespace adventure::engine
