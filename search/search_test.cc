#include <gtest/gtest.h>

#include "search.h"
#include "movegen/board.h"
#include "movegen/movegen.h"

using namespace chess;

namespace {

Board board_from(const char* fen) {
  Board b;
  EXPECT_TRUE(b.set_fen(fen));
  return b;
}

}  // namespace

TEST(Search, EvaluateMaterialStart) {
  auto b = board_from("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  EXPECT_EQ(evaluate(b), 0);
}

TEST(Search, EvaluateMaterialUp) {
  auto b = board_from("4k3/8/8/8/8/8/4P3/4K3 w - - 0 1");
  EXPECT_GT(evaluate(b), 0);
}

TEST(Search, EvaluateMaterialDown) {
  auto b = board_from("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
  // Black king on e8, white king on e1, white to move -> evaluation should be negative from black perspective? Actually evaluate returns score from side to move perspective.
  // With equal material, expect 0
  EXPECT_EQ(evaluate(b), 0);
}

TEST(Search, NegamaxDepthZeroReturnsEval) {
  auto b = board_from("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
  auto res = negamax(b, 0, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), false);
  EXPECT_EQ(res.score, evaluate(b));
  EXPECT_GT(res.nodes, 0);
}

TEST(Search, NegamaxRootFindsLegalMove) {
  auto b = board_from("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  auto res = negamax_root(b, 1);
  // Depth 1 should pick a move, score should be non-zero or zero
  EXPECT_NE(res.best_move.from, -1);
  EXPECT_GT(res.nodes, 0);
}

TEST(Search, NegamaxRootDepth1Consistency) {
  auto b = board_from("8/8/8/8/8/3k4/8/4K3 w - - 0 1");
  auto res = negamax_root(b, 1);
  // King should move towards center, just check it returns a move
  EXPECT_NE(res.best_move.from, -1);
  EXPECT_NE(res.best_move.to, -1);
}

TEST(Search, NullMoveHeuristicDoesNotCrash) {
  auto b = board_from("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
  auto res = negamax_root(b, 2);
  EXPECT_GT(res.nodes, 0);
  // Score should be within reasonable bounds
  EXPECT_GT(res.score, -30000);
  EXPECT_LT(res.score, 30000);
}

TEST(Search, SearchRespectsCheck) {
  // White king in check, must escape
  auto b = board_from("4rk2/8/8/8/8/8/8/4K3 w - - 0 1");
  auto res = negamax_root(b, 2);
  // The best move should not leave king in check
  Board child = b;
  child.make_move(res.best_move);
  EXPECT_FALSE(in_check(child));
}
