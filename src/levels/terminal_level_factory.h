#ifndef CLI_ADVENTURE_LEVELS_TERMINAL_LEVEL_FACTORY_H_
#define CLI_ADVENTURE_LEVELS_TERMINAL_LEVEL_FACTORY_H_

#include <memory>

#include "levels/ilevel.h"
#include "parser/parsed_level.h"
#include "ui/renderer.h"

namespace adventure::levels {

class TerminalLevelFactory {
 public:
  explicit TerminalLevelFactory(const adventure::ui::Renderer& renderer);

  std::unique_ptr<ILevel> create(const adventure::parser::ParsedLevelData& data) const;

 private:
  const adventure::ui::Renderer& renderer_;
};

}  // namespace adventure::levels

#endif  // CLI_ADVENTURE_LEVELS_TERMINAL_LEVEL_FACTORY_H_
