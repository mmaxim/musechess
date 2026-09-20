#include "search.h"
#include "eval/eval.h"

#include <limits>

namespace chess {

template <typename Evaluator>
Search<Evaluator>::Search(Evaluator eval) : evaluator_(std::move(eval)) {}

template <typename Evaluator>
SearchResult Search<Evaluator>::search(Board& b, int depth) {
  nodes_ = 0;
  auto best_score = std::numeric_limits<int>::min();
  Move best_move{};
  for (auto& m : generate_moves(b)) {
    Board child = b;
    child.make_move(m);
    auto score = -negamax_internal(child, depth - 1, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    if (score > best_score) {
      best_score = score;
      best_move = m;
    }
  }
  return {best_score, best_move, nodes_};
}

template <typename Evaluator>
int Search<Evaluator>::negamax_internal(Board& b, int depth, int alpha, int beta) {
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

  auto best = std::numeric_limits<int>::min();
  for (auto& m : moves) {
    Board child = b;
    child.make_move(m);
    auto score = -negamax_internal(child, depth - 1, -beta, -alpha);
    if (score > best) best = score;
    if (score > alpha) alpha = score;
    if (alpha >= beta) break;
  }
  return best;
}

// Explicit instantiation for the evaluators we know about
template class Search<class MaterialEvaluator>;

}  // namespace chess
