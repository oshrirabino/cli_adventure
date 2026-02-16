#ifndef CLI_ADVENTURE_CONTEXT_GAME_CONTEXT_H_
#define CLI_ADVENTURE_CONTEXT_GAME_CONTEXT_H_

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace adventure::context {

class GameContext {
 public:
  GameContext() = default;

  const std::string& current_directory() const;
  void set_current_directory(std::string directory);

  const std::string& current_level_path() const;
  void set_current_level_path(std::string path);

  bool has_next_level_request() const;
  const std::string& next_level_request() const;
  void request_next_level(std::string relative_path);
  void clear_next_level_request();

  bool is_game_over() const;
  void set_game_over(bool value);

  bool is_victory() const;
  void set_victory(bool value);

  const std::unordered_map<std::string, std::string>& memory_values() const;
  bool has_memory_value(const std::string& key) const;
  const std::string* get_memory_value(const std::string& key) const;
  void set_memory_value(std::string key, std::string value);
  void erase_memory_value(const std::string& key);

  const std::unordered_set<std::string>& memory_flags() const;
  bool has_memory_flag(const std::string& flag) const;
  void set_memory_flag(std::string flag);
  void clear_memory_flag(const std::string& flag);

 private:
  std::string current_directory_;
  std::string current_level_path_;
  std::string next_level_request_;
  bool game_over_ = false;
  bool victory_ = false;
  std::unordered_map<std::string, std::string> memory_values_;
  std::unordered_set<std::string> memory_flags_;
};

}  // namespace adventure::context

#endif  // CLI_ADVENTURE_CONTEXT_GAME_CONTEXT_H_
