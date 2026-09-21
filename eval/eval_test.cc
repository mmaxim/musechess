#include <gtest/gtest.h>

#include "eval.h"
#include "movegen/board.h"

using namespace chess;

TEST(Eval, StartPositionIsZero) {
  Board b;
  ASSERT_TRUE(b.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
  MaterialEvaluator eval;
  EXPECT_EQ(eval.evaluate(b), 0);
}

TEST(Eval, MaterialUp) {
  Board b;
  ASSERT_TRUE(b.set_fen("4k3/8/8/8/8/8/4P3/4K3 w - - 0 1"));
  MaterialEvaluator eval;
  EXPECT_GT(eval.evaluate(b), 0);
}

TEST(Eval, PawnAdvancementImproves) {
  Board b;
  ASSERT_TRUE(b.set_fen("4k3/8/8/8/8/8/PPPPPPPP/4K3 w - - 0 1"));
  MaterialEvaluator eval;
  int before = eval.evaluate(b);
  Board after = b;
  Move m;
  m.from = 8; // a2
  m.to = 16;  // a3
  after.make_move(m);
  int after_score = eval.evaluate(after);
  // After white move, evaluation from black perspective is negative of white gain
  EXPECT_LT(after_score, before);
}

TEST(Eval, CentralKnightBetterThanEdge) {
  Board b;
  ASSERT_TRUE(b.set_fen("4k3/8/8/8/3N4/8/8/4K3 w - - 0 1"));
  MaterialEvaluator eval;
  int central = eval.evaluate(b);
  Board b2;
  ASSERT_TRUE(b2.set_fen("4k3/8/8/8/8/8/8/4K3 w - - 0 1"));
  b2.masks[kWhiteKnight] = bitboard::set_bit(0); // a1
  int edge = eval.evaluate(b2);
  EXPECT_GT(central, edge);
}

TEST(Eval, QueenVsPawn) {
  Board b;
  ASSERT_TRUE(b.set_fen("4k3/8/8/8/8/8/8/4K3 w - - 0 1"));
  b.masks[kWhiteQueen] = bitboard::set_bit(4); // e1? Actually 4 is e1? Wait square 4 is e1? 0=a1, 4=e1. okay.
  MaterialEvaluator eval;
  int with_queen = eval.evaluate(b);
  Board b2;
  ASSERT_TRUE(b2.set_fen("4k3/8/8/8/8/8/8/4K3 w - - 0 1"));
  b2.masks[kWhitePawn] = bitboard::set_bit(4);
  int with_pawn = eval.evaluate(b2);
  EXPECT_GT(with_queen, with_pawn);
}
