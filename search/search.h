#pragma once

#include "movegen/board.h"
#include "movegen/move.h"
#include "movegen/movegen.h"

#include <functional>
#include <limits>
#include <vector>

namespace chess {

struct SearchResult {
  int score = 0;
  Move best_move{};
  int nodes = 0;
  std::vector<Move> pv;
};

template <typename Evaluator>
class Search {
 public:
  explicit Search(Evaluator eval);

  SearchResult search(Board& b, int depth);

 private:
  Evaluator evaluator_;
  int nodes_ = 0;

  int negamax_internal(Board& b, int depth, int alpha, int beta);
};

}  // namespace chess
