#include "uci.h"
#include <memory>
#include "movegen/board.h"
#include "game/game.h"
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
  Clock game_clock;
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

void Engine::set_time_control(const std::string& key, const std::string& value) {
  try {
    int val = std::stoi(value);
    auto ms = std::chrono::milliseconds(val);
    if (key == "wtime") impl_->game_clock.set_time(Color::White, ms);
    else if (key == "btime") impl_->game_clock.set_time(Color::Black, ms);
    else if (key == "winc") impl_->game_clock.set_inc(Color::White, ms);
    else if (key == "binc") impl_->game_clock.set_inc(Color::Black, ms);
  } catch (...) {}
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
  {
    auto move_start = std::chrono::steady_clock::now();
    
    // Decide time limit for this move.
    std::chrono::milliseconds time_limit{10000}; // Default 10s
    if (impl_->board.side_to_move == Color::White) {
      auto remaining = impl_->game_clock.white_time;
      if (remaining > std::chrono::milliseconds(0)) {
        time_limit = std::min(std::chrono::milliseconds(5000), remaining / 10);
      }
    } else {
      auto remaining = impl_->game_clock.black_time;
      if (remaining > std::chrono::milliseconds(0)) {
        time_limit = std::min(std::chrono::milliseconds(5000), remaining / 10);
      }
    }

    auto time_up_cb = [&move_start, time_limit]() {
      auto now = std::chrono::steady_clock::now();
      return std::chrono::duration_cast<std::chrono::milliseconds>(now - move_start) >= time_limit;
    };

    if (infinite) {
      // Run iterative deepening in a background thread so stop/quit can be processed
      if (impl_->search_thread.joinable()) {
        impl_->search_thread.join();
      }
      impl_->search_thread = std::thread([this, send_info, time_up_cb](){ 
        int d = 1;
        while (!impl_->stopped) {
          // Use a local copy of board to avoid data race with position updates
          Board board_copy;
          {
            std::lock_guard<std::mutex> lk(impl_->mtx);
            board_copy = impl_->board;
          }
          auto res = impl_->deepener.search_depth(board_copy, d, [&](const SearchResult& partial){
            send_info(d, partial);
          }, time_up_cb);
          {
            std::lock_guard<std::mutex> lk(impl_->mtx);
            if (res.best_move.from != -1) {
              impl_->last_best_move = res.best_move.to_string();
            }
          }
          ++d;
        }
      });
      return;
    }
    for (int d = 1; d <= maxDepth; ++d) {
      auto res = impl_->deepener.search_depth(impl_->board, d, [&](const SearchResult& partial){
        send_info(d, partial);
      }, time_up_cb);
      if (res.best_move.from != -1) {
        impl_->last_best_move = res.best_move.to_string();
      }
    }
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
