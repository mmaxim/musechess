#include "search.h"

#include <limits>

namespace chess {

namespace {

// Simple material table.
constexpr int kMaterial[12] = {
    100,  // WhitePawn
    320,  // WhiteKnight
    330,  // WhiteBishop
    500,  // WhiteRook
    900,  // WhiteQueen
    20000, // WhiteKing
    100,  // BlackPawn
    320,  // BlackKnight
    330,  // BlackBishop
    500,  // BlackRook
    900,  // BlackQueen
    20000 // BlackKing
};

int piece_value(Piece p) {
  return kMaterial[static_cast<int>(p)];
}

}  // namespace

int evaluate(const Board& b) {
  auto score = 0;
  for (auto i = 0; i < kNumPieces; ++i) {
    auto mask = b.masks[i];
    while (mask) {
      mask &= mask - 1;
      score += (color_of_piece(static_cast<Piece>(i)) == Color::White) ? piece_value(static_cast<Piece>(i))
                                                                        : -piece_value(static_cast<Piece>(i));
    }
  }
  return b.side_to_move == Color::White ? score : -score;
}

namespace {

int negamax_internal(Board& b, int depth, int alpha, int beta, bool can_null, int& nodes) {
  ++nodes;
  if (depth == 0) {
    return evaluate(b);
  }

  // Null move heuristic
  if (can_null && depth >= 2 && !in_check(b)) {
    auto saved_side = b.side_to_move;
    b.side_to_move = opponent(saved_side);
    // Reduce depth
    int R = 2;
    int score = -negamax_internal(b, depth - R - 1, -beta, -beta + 1, false, nodes);
    b.side_to_move = saved_side;
    if (score >= beta) {
      return beta;
    }
  }

  auto best = std::numeric_limits<int>::min();
  for (auto& m : chess::generate_moves(b)) {
    Board child = b;
    child.make_move(m);
    auto score = -negamax_internal(child, depth - 1, -beta, -alpha, true, nodes);
    if (score > best) best = score;
    if (score > alpha) alpha = score;
    if (alpha >= beta) break;
  }
  return best;
}

}  // namespace

SearchResult negamax(Board& b, int depth, int alpha, int beta, bool can_null) {
  int nodes = 0;
  auto score = negamax_internal(b, depth, alpha, beta, can_null, nodes);
  return {score, Move{}, nodes};
}

SearchResult negamax_root(Board& b, int depth) {
  auto best_score = std::numeric_limits<int>::min();
  Move best_move{};
  int best_nodes = 0;

  for (auto& m : chess::generate_moves(b)) {
    Board child = b;
    child.make_move(m);
    int nodes = 0;
    auto score = -negamax_internal(child, depth - 1, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), true, nodes);
    best_nodes += nodes;
    if (score > best_score) {
      best_score = score;
      best_move = m;
    }
  }
  return {best_score, best_move, best_nodes};
}

}  // namespace chess
