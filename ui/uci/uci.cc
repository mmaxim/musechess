#include "uci.h"
#include <memory>
#include "movegen/board.h"
#include "search/iterative_deepener.h"
#include "movegen/movegen.h"
#include "search/search.h"
#include "eval/eval.h"

#include <sstream>
#include <iostream>

namespace chess {
namespace uci {

class Engine::Impl {
 public:
  Board board;
  Options opts;
  IterativeDeepener<MaterialEvaluator> deepener{MaterialEvaluator{}};
  std::string last_best_move;
  bool stopped = false;
};

Engine::Engine() : impl_(std::make_unique<Impl>()) {}
Engine::~Engine() = default;

void Engine::set_option(const std::string& key, const std::string& value) {
  if (key == "Depth") {
    try { impl_->opts.depth = std::stoi(value); } catch (...) {}
  }
}

void Engine::uci_new_game() {
  impl_->board.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  impl_->last_best_move.clear();
}

void Engine::position(const std::string& fen, const std::vector<std::string>& moves) {
  Board b;
  if (!fen.empty()) {
    b.set_fen(fen);
  } else {
    b.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  }
  for (auto& mstr : moves) {
    // Very simple UCI move parser: e2e4
    if (mstr.size() < 4) continue;
    int from = (mstr[0]-'a') + (mstr[1]-'1')*8;
    int to   = (mstr[2]-'a') + (mstr[3]-'1')*8;
    Move m; m.from = from; m.to = to;
    b.make_move(m);
  }
  impl_->board = b;
}

void Engine::go(int depth, bool infinite, std::function<void(const std::string&)> info_cb) {
  impl_->stopped = false;
  int maxDepth = depth > 0 ? depth : impl_->opts.depth;
  auto output_info = [&](int d, int score, int nodes, Move best){
    if (!impl_->stopped) {
      std::string s = "info depth " + std::to_string(d) + " score cp " + std::to_string(score) + " nodes " + std::to_string(nodes) + " pv " + best.to_string();
      if (info_cb) info_cb(s);
      else {
        std::cout << s << "\n";
        std::cout.flush();
      }
    }
  };
  if (infinite) {
    int d = 1;
    while (!impl_->stopped) {
      auto res = impl_->deepener.search_depth(impl_->board, d, output_info);
      impl_->last_best_move = res.best_move.to_string();
      ++d;
    }
    return;
  }
  auto res = impl_->deepener.search(impl_->board, maxDepth, output_info);
  impl_->last_best_move = res.best_move.to_string();
}

void Engine::stop() {
  impl_->stopped = true;
}

std::string Engine::best_move() const {
  return impl_->last_best_move;
}

std::string Engine::info() const {
  return "info depth ...";
}

}  // namespace uci
}  // namespace chess
