#include "context/game_context.h"

namespace adventure::context {

const std::string& GameContext::current_directory() const { return current_directory_; }

void GameContext::set_current_directory(std::string directory) {
  current_directory_ = std::move(directory);
}

const std::string& GameContext::current_level_path() const { return current_level_path_; }

void GameContext::set_current_level_path(std::string path) {
  current_level_path_ = std::move(path);
}

bool GameContext::has_next_level_request() const { return !next_level_request_.empty(); }

const std::string& GameContext::next_level_request() const { return next_level_request_; }

void GameContext::request_next_level(std::string relative_path) {
  next_level_request_ = std::move(relative_path);
}

void GameContext::clear_next_level_request() { next_level_request_.clear(); }

bool GameContext::is_game_over() const { return game_over_; }

void GameContext::set_game_over(bool value) { game_over_ = value; }

bool GameContext::is_victory() const { return victory_; }

void GameContext::set_victory(bool value) { victory_ = value; }

const std::unordered_map<std::string, std::string>& GameContext::memory_values() const {
  return memory_values_;
}

bool GameContext::has_memory_value(const std::string& key) const {
  return memory_values_.find(key) != memory_values_.end();
}

const std::string* GameContext::get_memory_value(const std::string& key) const {
  const auto it = memory_values_.find(key);
  if (it == memory_values_.end()) {
    return nullptr;
  }
  return &it->second;
}

void GameContext::set_memory_value(std::string key, std::string value) {
  memory_values_[std::move(key)] = std::move(value);
}

void GameContext::erase_memory_value(const std::string& key) { memory_values_.erase(key); }

const std::unordered_set<std::string>& GameContext::memory_flags() const {
  return memory_flags_;
}

bool GameContext::has_memory_flag(const std::string& flag) const {
  return memory_flags_.find(flag) != memory_flags_.end();
}

void GameContext::set_memory_flag(std::string flag) { memory_flags_.insert(std::move(flag)); }

void GameContext::clear_memory_flag(const std::string& flag) { memory_flags_.erase(flag); }

}  // namespace adventure::context
