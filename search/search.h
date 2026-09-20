#pragma once

#include "movegen/board.h"
#include "movegen/move.h"
#include "movegen/movegen.h"

namespace chess {

struct SearchResult {
  int score = 0;
  Move best_move{};
  int nodes = 0;
};

int evaluate(const Board& b);

SearchResult negamax_root(Board& b, int depth);

SearchResult negamax(Board& b, int depth, int alpha, int beta, bool can_null);

}  // namespace chess
