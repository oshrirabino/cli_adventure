#ifndef CLI_ADVENTURE_VALIDATION_GAME_VALIDATOR_H_
#define CLI_ADVENTURE_VALIDATION_GAME_VALIDATOR_H_

#include <filesystem>
#include <string>
#include <vector>

namespace adventure::validation {

struct ValidationIssue {
  std::filesystem::path file;
  std::string message;
};

struct ValidationReport {
  std::size_t checked_files = 0;
  std::vector<ValidationIssue> issues;
};

ValidationReport validate_game(const std::filesystem::path& game_root);

}  // namespace adventure::validation

#endif  // CLI_ADVENTURE_VALIDATION_GAME_VALIDATOR_H_
