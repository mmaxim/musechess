#include "search.h"
#include "eval/eval.h"

#include <limits>

namespace chess {

template <typename Evaluator>
Search<Evaluator>::Search(Evaluator eval) : evaluator_(std::move(eval)) {}

template <typename Evaluator>
SearchResult Search<Evaluator>::search(Board& b, int depth) {
  auto best_score = std::numeric_limits<int>::min();
  Move best_move{};
  nodes_ = 0;

  for (auto& m : generate_moves(b)) {
    Board child = b;
    child.make_move(m);
    auto score = -negamax_internal(child, depth - 1, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), true);
    if (score > best_score) {
      best_score = score;
      best_move = m;
    }
  }
  return {best_score, best_move, nodes_};
}

template <typename Evaluator>
int Search<Evaluator>::negamax_internal(Board& b, int depth, int alpha, int beta, bool can_null) {
  ++nodes_;
  if (depth == 0) {
    return evaluator_.evaluate(b);
  }

  if (can_null && depth >= 2 && !in_check(b)) {
    auto saved_side = b.side_to_move;
    b.side_to_move = opponent(saved_side);
    auto R = 2;
    auto score = -negamax_internal(b, depth - R - 1, -beta, -beta + 1, false);
    b.side_to_move = saved_side;
    if (score >= beta) {
      return beta;
    }
  }

  auto best = std::numeric_limits<int>::min();
  for (auto& m : generate_moves(b)) {
    Board child = b;
    child.make_move(m);
    auto score = -negamax_internal(child, depth - 1, -beta, -alpha, true);
    if (score > best) best = score;
    if (score > alpha) alpha = score;
    if (alpha >= beta) break;
  }
  return best;
}

// Explicit instantiation for the evaluators we know about
template class Search<class MaterialEvaluator>;

}  // namespace chess
