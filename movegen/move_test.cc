#include <gtest/gtest.h>
#include "board.h"
#include "move.h"
#include "movegen.h"

using namespace chess;

namespace {
int sq_to_idx(char file, char rank) {
  return (rank - '1') * 8 + (file - 'a');
}

Board board_from(const char* fen) {
  Board b;
  if (!b.set_fen(fen)) {
    throw std::runtime_error("Failed to set FEN");
  }
  return b;
}
}

TEST(MoveSAN, SimpleMoves) {
  Board b = board_from("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  Move m1{sq_to_idx('e', '2'), sq_to_idx('e', '4')};
  EXPECT_EQ(m1.to_san(b), "e4");
  
  Move m2{sq_to_idx('g', '1'), sq_to_idx('f', '3')};
  EXPECT_EQ(m2.to_san(b), "Nf3");
}

TEST(MoveSAN, Capture) {
  Board b = board_from("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  b.make_move({sq_to_idx('e', '2'), sq_to_idx('e', '4')});
  Move m{sq_to_idx('d', '7'), sq_to_idx('e', '4')};
  EXPECT_EQ(m.to_san(b), "dxe4");
}

TEST(MoveSAN, DisambiguationKnight) {
  // Knights on b1 and f3. Both can move to d2.
  // Add kings to make FEN valid.
  Board b = board_from("8/8/8/8/8/5N2/3P4/1N6 K1 - - 0 1"); 
  // Wait, "K1" is not a valid FEN. Let's use a proper one.
  // 8/8/8/8/8/5N2/3P4/1N6 w - - 0 1 -> add kings.
  // k on e8, K on e1.
  Board b2 = board_from("8/4k3/8/8/8/5N2/3P4/1N4K1 w - - 0 1");
  Move m1{sq_to_idx('b', '1'), sq_to_idx('d', '2')};
  Move m2{sq_to_idx('f', '3'), sq_to_idx('d', '2')};
  
  EXPECT_EQ(m1.to_san(b2), "Nbd2");
  EXPECT_EQ(m2.to_san(b2), "Nfd2");
}

TEST(MoveSAN, DisambiguationKnightSameFile) {
  // Knights on b1 and b3. Both can move to d2.
  Board b = board_from("8/4k3/8/8/8/1N4/3P4/1N4K1 w - - 0 1");
  Move m1{sq_to_idx('b', '1'), sq_to_idx('d', '2')};
  Move m2{sq_to_idx('b', '3'), sq_to_idx('d', '2')};
  
  EXPECT_EQ(m1.to_san(b), "N1d2");
  EXPECT_EQ(m2.to_san(b), "N3d2");
}

TEST(MoveSAN, DisambiguationRook) {
  Board b = board_from("8/4k3/8/8/8/8/8/R3K2R w - - 0 1");
  Move m1{sq_to_idx('a', '1'), sq_to_idx('e', '1')};
  Move m2{sq_to_idx('h', '1'), sq_to_idx('e', '1')};
  
  EXPECT_EQ(m1.to_san(b), "Rae1");
  EXPECT_EQ(m2.to_san(b), "Rhe1");
}

TEST(MoveSAN, DisambiguationBishop) {
  Board b = board_from("8/4k3/8/8/8/3B4/8/4K1B1 w - - 0 1");
  Move m1{sq_to_idx('f', '1'), sq_to_idx('e', '2')};
  Move m2{sq_to_idx('d', '3'), sq_to_idx('e', '2')};
  
  EXPECT_EQ(m1.to_san(b), "Bfe2");
  EXPECT_EQ(m2.to_san(b), "Bde2");
}

TEST(MoveSAN, Castling) {
  Board b = board_from("7k/8/8/8/8/8/8/4K2R w K - 0 1");
  Move m{sq_to_idx('e', '1'), sq_to_idx('g', '1')};
  m.flags |= Move::kCastling;
  EXPECT_EQ(m.to_san(b), "O-O");
}
