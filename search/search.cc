#include "search.h"
#include "eval/eval.h"

#include <limits>

namespace chess {

template <typename Evaluator>
Search<Evaluator>::Search(Evaluator eval) : evaluator_(std::move(eval)) {}

template <typename Evaluator>
SearchResult Search<Evaluator>::search(Board& b, int depth) {
  nodes_ = 0;
  Variation pv;
  pv.moves.resize(kMaxDepth);
  int alpha = std::numeric_limits<int>::min();
  int beta = std::numeric_limits<int>::max();
  int score = negamax_internal(b, depth, alpha, beta, pv, 0);
  SearchResult res;
  res.score = score;
  res.nodes = nodes_;
  if (!pv.moves.empty() && pv.moves[0].from != -1) {
    res.best_move = pv.moves[0];
    for (int i = 0; i < depth && i < static_cast<int>(pv.moves.size()) && pv.moves[i].from != -1; ++i) {
      res.pv.add(pv.moves[i]);
    }
  }
  return res;
}

template <typename Evaluator>
int Search<Evaluator>::negamax_internal(Board& b, int depth, int alpha, int beta, Variation& pv, int ply) {
  if (stop_flag_ && stop_flag_->load()) {
    return 0;
  }
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
    if (stop_flag_ && stop_flag_->load()) break;
    Board child = b;
    child.make_move(m);
    Variation child_pv;
    child_pv.moves.resize(kMaxDepth);
    int score = -negamax_internal(child, depth - 1, -beta, -alpha, child_pv, ply + 1);
    if (score > best) {
      best = score;
      best_move = m;
      // copy child's PV into current PV slots
      for (int i = 0; i < depth - 1; ++i) {
        if (ply + 1 + i < static_cast<int>(pv.moves.size())) {
          pv.moves[ply + 1 + i] = child_pv.moves[ply + 1 + i];
        }
      }
    }
    if (score > alpha) alpha = score;
    if (alpha >= beta) break;
  }
  if (ply < static_cast<int>(pv.moves.size())) {
    pv.moves[ply] = best_move;
  }
  return best;
}

// Explicit instantiation for the evaluators we know about
template class Search<class MaterialEvaluator>;

}  // namespace chess
