#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace chess {
namespace uci {

struct Options {
  int depth = 12;
};

class Engine {
 public:
  Engine();
  ~Engine();
  void set_option(const std::string& key, const std::string& value);
  void uci_new_game();
  void position(const std::string& fen, const std::vector<std::string>& moves);
  void go(int depth = -1, bool infinite = false, std::function<void(const std::string&)> info_cb = nullptr);
  void stop();
  std::string best_move() const;
  std::string info() const;

 private:
  // Internal state
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace uci
}  // namespace chess
