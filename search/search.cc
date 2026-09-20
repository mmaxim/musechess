#include "search.h"
#include "eval/eval.h"

#include <limits>

namespace chess {

template <typename Evaluator>
Search<Evaluator>::Search(Evaluator eval) : evaluator_(std::move(eval)) {}

template <typename Evaluator>
SearchResult Search<Evaluator>::search(Board& b, int depth) {
  nodes_ = 0;
  Move pv[kMaxDepth];
  int alpha = std::numeric_limits<int>::min();
  int beta = std::numeric_limits<int>::max();
  int score = negamax_internal(b, depth, alpha, beta, pv, 0);
  SearchResult res;
  res.score = score;
  res.nodes = nodes_;
  if (pv[0].from != -1) {
    res.best_move = pv[0];
    for (int i = 0; i < depth && pv[i].from != -1; ++i) {
      res.pv.add(pv[i]);
    }
  }
  return res;
}

template <typename Evaluator>
int Search<Evaluator>::negamax_internal(Board& b, int depth, int alpha, int beta, Move* pv, int ply) {
  ++nodes_;
  if (depth == 0) {
    return evaluator_.evaluate(b);
  }

  auto moves = generate_moves(b);
  if (moves.empty()) {
    if (in_check(b)) {
      return -100000 + depth;
    }
    return 0;
  }

  int best = std::numeric_limits<int>::min();
  Move best_move{};
  for (auto& m : moves) {
    Board child = b;
    child.make_move(m);
    Move child_pv[kMaxDepth];
    int score = -negamax_internal(child, depth - 1, -beta, -alpha, child_pv, ply + 1);
    if (score > best) {
      best = score;
      best_move = m;
      // copy child PV into current PV
      for (int i = 0; i < kMaxDepth - ply - 1; ++i) {
        pv[ply + 1 + i] = child_pv[ply + 1 + i];
      }
    }
    if (score > alpha) alpha = score;
    if (alpha >= beta) break;
  }
  pv[ply] = best_move;
  return best;
}

// Explicit instantiation for the evaluators we know about
template class Search<class MaterialEvaluator>;

}  // namespace chess
