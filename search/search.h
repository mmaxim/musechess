#pragma once

#include "movegen/board.h"
#include "movegen/move.h"
#include "movegen/movegen.h"

#include <functional>
#include <limits>
#include <vector>
#include <atomic>
#include <climits>

namespace chess {

struct Variation {
  std::vector<Move> moves;
  void add(const Move& m) { moves.push_back(m); }
  void clear() { moves.clear(); }
  void set(int ply, const Move& m) {
    if (moves.size() <= static_cast<size_t>(ply)) moves.resize(ply + 1);
    moves[ply] = m;
  }
  void merge_child(const Variation& child, int ply) {
    // Copy child's PV from ply+1 onward into this variation.
    for (size_t i = static_cast<size_t>(ply + 1); i < child.moves.size(); ++i) {
      const Move& m = child.moves[i];
      if (m.from == -1) break;
      if (moves.size() <= i) moves.resize(i + 1);
      moves[i] = m;
    }
  }
  std::string to_string() const {
    std::string s;
    for (size_t i = 0; i < moves.size(); ++i) {
      s += moves[i].to_string();
      if (i + 1 < moves.size()) s += ' ';
    }
    return s;
  }
};

struct SearchResult {
  int score = 0;
  Move best_move{};
  int nodes = 0;
  Variation pv;
};

template <typename Evaluator>
class Search {
 public:
  explicit Search(Evaluator eval);

  SearchResult search(Board& b, int depth, std::function<void(const SearchResult&)> cb = nullptr);

  void set_stop_flag(std::atomic<bool>* flag) { stop_flag_ = flag; }

 private:
  Evaluator evaluator_;
  int nodes_ = 0;
  static constexpr int kMaxDepth = 64;
  std::atomic<bool>* stop_flag_ = nullptr;
  std::function<void(const SearchResult&)> callback_ = nullptr;

  int negamax_internal(Board& b, int depth, int alpha, int beta, Variation& pv, int ply);
  int quiescent(Board& b, int alpha, int beta);
};

}  // namespace chess
