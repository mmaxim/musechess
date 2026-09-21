#include "search.h"
#include "eval/eval.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <vector>

namespace chess {

template <typename Evaluator>
Search<Evaluator>::Search(Evaluator eval) : evaluator_(std::move(eval)) {}

template <typename Evaluator>
SearchResult Search<Evaluator>::search(Board& b, int depth, std::function<void(const SearchResult&)> cb, std::function<bool()> time_up_cb) {
  callback_ = std::move(cb);
  nodes_ = 0;
  Variation pv;
  pv.moves.resize(kMaxDepth);
  int alpha = -1000000;
  int beta = 1000000;
  int score = negamax_internal(b, depth, alpha, beta, pv, 0, time_up_cb);
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
int Search<Evaluator>::negamax_internal(Board& b, int depth, int alpha, int beta, Variation& pv, int ply, std::function<bool()> time_up_cb) {
  if ((stop_flag_ && stop_flag_->load()) || (time_up_cb && time_up_cb())) {
    return 0;
  }
  ++nodes_;
  if (depth <= 0) {
    return quiescent(b, alpha, beta, time_up_cb);
  }

  auto moves = generate_moves(b);
  if (ply == 0) {
    std::cerr << "Root moves: " << moves.size() << std::endl;
  }
  if (moves.empty()) {
    if (in_check(b)) {
      return -100000 + depth;
    }
    return 0;
  }

  // Simple move ordering: captures first, then promotions.
  std::vector<Move> ordered;
  ordered.reserve(moves.size());
  for (auto& m : moves) ordered.push_back(m);
  auto me = b.side_to_move;
  auto opp = opponent(me);
  auto score_move = [&](const Move& m) -> int {
    int score = 0;
    bool is_capture = false;
    if (m.has_flag(Move::kEnPassant)) {
      is_capture = true;
    } else {
      auto to_piece = b.piece_at(m.to);
      if (to_piece != chess::kNumPieces && color_of_piece(to_piece) == opp) {
        is_capture = true;
      }
    }
    if (m.promotion != PieceType::None) {
      score += 100000;
    }
    if (is_capture) {
      auto captured_piece = b.piece_at(m.to);
      if (m.has_flag(Move::kEnPassant)) {
        captured_piece = (me == Color::White) ? kBlackPawn : kWhitePawn;
      }
      auto mover_piece = b.piece_at(m.from);
      int captured_val = 0;
      int mover_val = 0;
      auto vals = std::array<int,6>{100, 320, 330, 500, 900, 20000};
      if (captured_piece != chess::kNumPieces) captured_val = vals[static_cast<int>(type_of_piece(captured_piece))];
      if (mover_piece != chess::kNumPieces) mover_val = vals[static_cast<int>(type_of_piece(mover_piece))];
      score += 10000 + captured_val - mover_val;
    }
    // Slight preference for central moves
    {
      int r = bitboard::rank_of(m.to);
      int f = bitboard::file_of(m.to);
      int centrality = 3 - std::abs(r - 3) - std::abs(f - 3);
      score += centrality;
    }
    return score;
  };
  std::sort(ordered.begin(), ordered.end(), [&](const Move& a, const Move& b){
    return score_move(a) > score_move(b);
  });

  int best = std::numeric_limits<int>::min();
  Move best_move{};
  for (auto& m : ordered) {
    if (ply == 0) {
      std::cerr << "Trying move " << m.to_string() << std::endl;
    }
    if (stop_flag_ && stop_flag_->load()) break;
    Board child = b;
    child.make_move(m);
    Variation child_pv;
    child_pv.moves.resize(kMaxDepth);
    int score = -negamax_internal(child, depth - 1, -beta, -alpha, child_pv, ply + 1, time_up_cb);
    if (ply == 0) {
      std::cerr << "Score for " << m.to_string() << " = " << score << std::endl;
    }
    if (score > best) {
      best = score;
      best_move = m;
      // Merge child's PV into current variation using Variation helper
      pv.merge_child(child_pv, ply);
      pv.set(ply, best_move);
      if (ply == 0 && callback_) {
        SearchResult tmp;
        tmp.score = best;
        tmp.best_move = best_move;
        tmp.nodes = nodes_;
        // Build a shallow PV for the callback
        Variation cb_pv;
        for (int i = 0; i < depth && i < static_cast<int>(pv.moves.size()); ++i) {
          if (pv.moves[i].from != -1) cb_pv.add(pv.moves[i]);
        }
        tmp.pv = cb_pv;
        callback_(tmp);
      }
    }
    if (score > alpha) alpha = score;
    if (alpha >= beta) break;
  }
  pv.set(ply, best_move);
  return best;
}

template <typename Evaluator>
int Search<Evaluator>::quiescent(Board& b, int alpha, int beta, std::function<bool()> time_up_cb) {
  if ((stop_flag_ && stop_flag_->load()) || (time_up_cb && time_up_cb())) return 0;
  ++nodes_;

  int stand_pat = evaluator_.evaluate(b);
  if (stand_pat >= beta) return beta;
  if (alpha < stand_pat) alpha = stand_pat;

  auto moves = generate_moves(b);
  auto opp = opponent(b.side_to_move);
  std::vector<Move> captures;
  for (const auto& m : moves) {
    bool is_capture = false;
    if (m.has_flag(Move::kEnPassant)) is_capture = true;
    else {
      auto p = b.piece_at(m.to);
      if (p != chess::kNumPieces && color_of_piece(p) == opp) is_capture = true;
    }
    if (is_capture || m.promotion != PieceType::None) captures.push_back(m);
  }

  if (captures.empty()) return alpha;

  for (const auto& m : captures) {
    Board child = b;
    child.make_move(m);
    int score = -quiescent(child, -beta, -alpha, time_up_cb);
    if (score > alpha) alpha = score;
    if (alpha >= beta) break;
  }
  return alpha;
}

// Explicit instantiation for the evaluators we know about
template class Search<class MaterialEvaluator>;

}  // namespace chess
