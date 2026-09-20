#pragma once

#include "movegen/board.h"
#include "movegen/move.h"
#include "movegen/movegen.h"

#include <functional>
#include <limits>
#include <vector>

namespace chess {

struct PV {
  std::vector<Move> moves;
  void add(const Move& m) { moves.push_back(m); }
  void clear() { moves.clear(); }
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
  PV pv;
};

template <typename Evaluator>
class Search {
 public:
  explicit Search(Evaluator eval);

  SearchResult search(Board& b, int depth);

 private:
  Evaluator evaluator_;
  int nodes_ = 0;
  static constexpr int kMaxDepth = 64;

  int negamax_internal(Board& b, int depth, int alpha, int beta, Move* pv, int ply);
};

}  // namespace chess
