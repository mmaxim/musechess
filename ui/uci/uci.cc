#include "uci.h"
#include <memory>
#include "movegen/board.h"
#include "search/iterative_deepener.h"
#include "movegen/movegen.h"
#include "search/search.h"
#include "eval/eval.h"

#include <sstream>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

namespace chess {
namespace uci {

class Engine::Impl {
 public:
  Board board;
  Options opts;
  IterativeDeepener<MaterialEvaluator> deepener{MaterialEvaluator{}};
  std::string last_best_move;
  std::atomic<bool> stopped{false};
  std::thread search_thread;
  std::mutex mtx;
  std::function<void(const std::string&)> info_cb;
};

Engine::Engine() : impl_(std::make_unique<Impl>()) {}
Engine::~Engine() {
  impl_->stopped = true;
  if (impl_->search_thread.joinable()) {
    impl_->search_thread.join();
  }
}

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
  impl_->info_cb = info_cb;
  impl_->deepener.set_stop_flag(&impl_->stopped);
  int maxDepth = depth > 0 ? depth : impl_->opts.depth;
  auto send_info = [&](int d, const SearchResult& res){
    if (!impl_->stopped) {
      std::string pv_str = res.pv.to_string();
      std::string s = "info depth " + std::to_string(d) + " score cp " + std::to_string(res.score) + " nodes " + std::to_string(res.nodes) + " pv " + pv_str;
      if (info_cb) info_cb(s);
      else {
        std::cout << s << "\n";
        std::cout.flush();
      }
    }
  };
  if (infinite) {
    // Run iterative deepening in a background thread so stop/quit can be processed
    if (impl_->search_thread.joinable()) {
      impl_->search_thread.join();
    }
    impl_->search_thread = std::thread([this, send_info](){
      int d = 1;
      while (!impl_->stopped) {
        // Use a local copy of board to avoid data race with position updates
        Board board_copy;
        {
          std::lock_guard<std::mutex> lk(impl_->mtx);
          board_copy = impl_->board;
        }
        auto res = impl_->deepener.search_depth(board_copy, d);
        {
          std::lock_guard<std::mutex> lk(impl_->mtx);
          impl_->last_best_move = res.best_move.to_string();
        }
        send_info(d, res);
        ++d;
      }
    });
    return;
  }
  for (int d = 1; d <= maxDepth; ++d) {
    auto res = impl_->deepener.search_depth(impl_->board, d);
    impl_->last_best_move = res.best_move.to_string();
    send_info(d, res);
  }
}

void Engine::stop() {
  impl_->stopped = true;
  if (impl_->search_thread.joinable()) {
    impl_->search_thread.join();
  }
}

std::string Engine::best_move() const {
  return impl_->last_best_move;
}

std::string Engine::info() const {
  return "info depth ...";
}

}  // namespace uci
}  // namespace chess
