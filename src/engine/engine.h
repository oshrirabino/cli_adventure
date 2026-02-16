#ifndef CLI_ADVENTURE_ENGINE_ENGINE_H_
#define CLI_ADVENTURE_ENGINE_ENGINE_H_

#include <istream>
#include <ostream>

#include "context/game_context.h"
#include "levels/terminal_level_factory.h"
#include "parser/tag_parser.h"
#include "ui/renderer.h"

namespace adventure::engine {

class Engine {
 public:
  Engine();
  explicit Engine(adventure::ui::Renderer renderer);

  void run(std::istream& in, std::ostream& out, adventure::context::GameContext& context);

 private:
  static std::string resolve_next_level_path(const std::string& current_level_path,
                                             const std::string& relative_or_absolute_next);

  adventure::ui::Renderer renderer_;
  adventure::parser::TagParser parser_;
  adventure::levels::TerminalLevelFactory factory_;
};

}  // namespace adventure::engine

#endif  // CLI_ADVENTURE_ENGINE_ENGINE_H_
