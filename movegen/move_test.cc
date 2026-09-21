#include <gtest/gtest.h>
#include "board.h"
#include "move.h"
#include "movegen.h"

using namespace chess;

namespace {
int sq_to_idx(char file, char rank) {
  return (rank - '1') * 8 + (file - 'a');
}
}

TEST(MoveSAN, SimpleMoves) {
  Board b;
  ASSERT_TRUE(b.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
  Move m1{sq_to_idx('e', '2'), sq_to_idx('e', '4')};
  EXPECT_EQ(m1.to_san(b), "e4");
  
  Move m2{sq_to_idx('g', '1'), sq_to_idx('f', '3')};
  EXPECT_EQ(m2.to_san(b), "Nf3");
}

TEST(MoveSAN, Capture) {
  Board b;
  ASSERT_TRUE(b.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
  b.make_move({sq_to_idx('e', '2'), sq_to_idx('e', '4')});
  Move m{sq_to_idx('d', '7'), sq_to_idx('e', '4')};
  EXPECT_EQ(m.to_san(b), "dxe4");
}

TEST(MoveSAN, DisambiguationKnight) {
  Board b;
  // Knights on b1 and f3. Both can move to d2.
  ASSERT_TRUE(b.set_fen("8/4k3/8/8/8/5N2/8/1N4K1 w - - 0 1"));
  Move m1{sq_to_idx('b', '1'), sq_to_idx('d', '2')};
  Move m2{sq_to_idx('f', '3'), sq_to_idx('d', '2')};
  
  EXPECT_EQ(m1.to_san(b), "Nbd2");
  EXPECT_EQ(m2.to_san(b), "Nfd2");
}

TEST(MoveSAN, DisambiguationKnightSameFile) {
  Board b;
  // Knights on b1 and b3. Both can move to d2.
  ASSERT_TRUE(b.set_fen("8/4k3/8/8/8/1N6/8/1N4K1 w - - 0 1"));
  Move m1{sq_to_idx('b', '1'), sq_to_idx('d', '2')};
  Move m2{sq_to_idx('b', '3'), sq_to_idx('d', '2')};
  
  EXPECT_EQ(m1.to_san(b), "N1d2");
  EXPECT_EQ(m2.to_san(b), "N3d2");
}

TEST(MoveSAN, DisambiguationRook) {
  Board b;
  ASSERT_TRUE(b.set_fen("8/4k3/8/8/8/8/1K6/R6R w - - 0 1"));
  Move m1{sq_to_idx('a', '1'), sq_to_idx('e', '1')};
  Move m2{sq_to_idx('h', '1'), sq_to_idx('e', '1')};
  
  EXPECT_EQ(m1.to_san(b), "Rae1+");
  EXPECT_EQ(m2.to_san(b), "Rhe1+");
}

TEST(MoveSAN, DisambiguationBishop) {
  Board b;
  ASSERT_TRUE(b.set_fen("8/4k3/8/8/8/8/8/1B1K1B2 w - - 0 1"));
  Move m1{sq_to_idx('b', '1'), sq_to_idx('d', '3')};
  Move m2{sq_to_idx('f', '1'), sq_to_idx('d', '3')};
  
  EXPECT_EQ(m1.to_san(b), "Bbd3");
  EXPECT_EQ(m2.to_san(b), "Bfd3");
}

TEST(MoveSAN, Castling) {
  Board b;
  ASSERT_TRUE(b.set_fen("7k/8/8/8/8/8/8/4K2R w K - 0 1"));
  Move m{sq_to_idx('e', '1'), sq_to_idx('g', '1')};
  m.flags |= Move::kCastling;
  EXPECT_EQ(m.to_san(b), "O-O");
}
