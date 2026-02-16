#ifndef CLI_ADVENTURE_LEVELS_ILEVEL_H_
#define CLI_ADVENTURE_LEVELS_ILEVEL_H_

#include <istream>
#include <ostream>

#include "context/game_context.h"

namespace adventure::levels {

class ILevel {
 public:
  virtual ~ILevel() = default;

  virtual void render(std::ostream& out,
                      const adventure::context::GameContext& context) const = 0;
  virtual void execute(std::istream& in, std::ostream& out,
                       adventure::context::GameContext& context) = 0;
};

}  // namespace adventure::levels

#endif  // CLI_ADVENTURE_LEVELS_ILEVEL_H_
