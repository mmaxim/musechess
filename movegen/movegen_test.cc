#include <gtest/gtest.h>

#include <cstdint>

#include "board.h"
#include "movegen.h"

using namespace chess;

namespace {

Board board_from(const char* fen) {
  Board b;
  EXPECT_TRUE(b.set_fen(fen));
  return b;
}

bool has_move(const MoveList& ml, int from, int to, PieceType promo = PieceType::None) {
  for (const Move& m : ml) {
    if (m.from == from && m.to == to && m.promotion == promo) return true;
  }
  return false;
}

std::uint64_t perft(const Board& b, int depth) {
  if (depth == 0) return 1;
  std::uint64_t nodes = 0;
  for (const Move& m : generate_moves(b)) {
    Board n = b;
    n.make_move(m);
    nodes += perft(n, depth - 1);
  }
  return nodes;
}

}  // namespace

TEST(MoveGen, StartPositionMoveCount) {
  Board b = board_from("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  EXPECT_EQ(generate_moves(b).size(), 20);
  EXPECT_EQ(generate_pseudo_legal(b).size(), 20);
}

TEST(MoveGen, StartPositionPerftDepth2Breakdown) {
  Board b = board_from("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  // Known perft depth 2 breakdown for start position (CPW)
  // e2e4 -> 20, e2e3 -> 20, etc. We just verify total 400.
  std::uint64_t total = 0;
  for (const Move& m : generate_moves(b)) {
    Board n = b;
    n.make_move(m);
    std::uint64_t cnt = 0;
    for (const Move& m2 : generate_moves(n)) ++cnt;
    total += cnt;
  }
  EXPECT_EQ(total, 400);
}

TEST(MoveGen, KiwiPetePerftDepth1) {
  Board b = board_from("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  EXPECT_EQ(generate_moves(b).size(), 48);
} 

TEST(MoveGen, KnightMoveGeneration) {
  // Knight c3; own pawn on a4 blocks one destination. White king e1 adds 5 moves.
  Board b = board_from("8/8/8/8/P7/2N5/8/4K2k w - - 0 1");
  const MoveList ml = generate_moves(b);
  EXPECT_TRUE(has_move(ml, 18, 8));    // c3a2
  EXPECT_TRUE(has_move(ml, 18, 33));   // c3b5
  EXPECT_TRUE(has_move(ml, 18, 35));   // c3d5
  EXPECT_TRUE(has_move(ml, 18, 28));   // c3e4
  EXPECT_TRUE(has_move(ml, 18, 12));   // c3e2
  EXPECT_TRUE(has_move(ml, 18, 1));    // c3b1
  EXPECT_TRUE(has_move(ml, 18, 3));    // c3d1
  EXPECT_FALSE(has_move(ml, 18, 24));  // c3a4 blocked by own pawn
  EXPECT_EQ(ml.size(), 13);  // 7 knight + 5 king + 1 pawn
}

TEST(MoveGen, KingMoveGeneration) {
  Board b = board_from("4k3/8/8/8/4K3/8/8/8 w - - 0 1");
  const MoveList ml = generate_moves(b);
  EXPECT_TRUE(has_move(ml, 28, 27));  // e4d4
  EXPECT_TRUE(has_move(ml, 28, 37));  // e4f5
  EXPECT_TRUE(has_move(ml, 28, 36));  // e4e5
  EXPECT_EQ(ml.size(), 8);
}

TEST(MoveGen, RookMoveGeneration) {
  // Rook e1: a1-d1 and f1-g1, blocked by own pawn on e2. King h1: g1, g2, h2.
  Board b = board_from("k7/8/8/8/8/8/4P3/4R2K w - - 0 1");
  const MoveList ml = generate_moves(b);
  EXPECT_TRUE(has_move(ml, 4, 0));     // e1a1
  EXPECT_TRUE(has_move(ml, 4, 3));     // e1d1
  EXPECT_TRUE(has_move(ml, 4, 6));     // e1g1
  EXPECT_FALSE(has_move(ml, 4, 12));   // e1e2 blocked by own pawn
  EXPECT_TRUE(has_move(ml, 7, 14));    // h1g2
  EXPECT_EQ(ml.size(), 11);  // 6 rook + 3 king + 2 pawn
}

TEST(MoveGen, BishopMoveGeneration) {
  // Bishop e3: rays until blocked; own king on h1 stops the SE ray at g1.
  Board b = board_from("7k/8/8/8/8/4B3/8/7K w - - 0 1");
  const MoveList ml = generate_moves(b);
  EXPECT_TRUE(has_move(ml, 20, 48));   // e3a7
  EXPECT_TRUE(has_move(ml, 20, 47));   // e3h6
  EXPECT_TRUE(has_move(ml, 20, 6));    // e3g1
  EXPECT_FALSE(has_move(ml, 20, 7));   // h1 own king
  EXPECT_EQ(ml.size(), 14);  // 11 bishop + 3 king
}

TEST(MoveGen, EnPassantMoveGeneration) {
  Board b = board_from("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1");
  const MoveList ml = generate_moves(b);
  EXPECT_TRUE(has_move(ml, 36, 43));   // e5xd6 e.p.
  EXPECT_EQ(ml.size(), 7);             // 1 ep + 1 pawn move + 5 king moves
}

TEST(MoveGen, CastlingKingside) {
  Board b = board_from("7k/8/8/8/8/8/8/4K2R w K - 0 1");
  const MoveList ml = generate_moves(b);
  EXPECT_TRUE(has_move(ml, 4, 6));  // e1g1
  EXPECT_EQ(ml.size(), 15);         // 5 king + 9 rook + 1 castling
}

TEST(MoveGen, CastlingQueenside) {
  Board b = board_from("7k/8/8/8/8/8/8/R3K3 w Q - 0 1");
  const MoveList ml = generate_moves(b);
  EXPECT_TRUE(has_move(ml, 4, 2));  // e1c1
  EXPECT_EQ(ml.size(), 16);         // 5 king + 10 rook + 1 castling
}

TEST(MoveGen, CastlingBlockedByOwnPiece) {
  Board b = board_from("7k/8/8/8/8/8/8/4KP1R w K - 0 1");
  const MoveList ml = generate_moves(b);
  EXPECT_FALSE(has_move(ml, 4, 6));
  EXPECT_EQ(ml.size(), 13);  // 4 king + 9 rook + 1 pawn
}

TEST(MoveGen, CastlingBlockedByAttackedSquare) {
  // Black rook g1 checks the white king on e1 (via f1).
  Board b = board_from("k7/8/8/8/8/8/8/4K1rR w K - 0 1");
  EXPECT_TRUE(in_check(b));
  const MoveList ml = generate_moves(b);
  EXPECT_FALSE(has_move(ml, 4, 6));    // cannot castle out of check
  EXPECT_TRUE(has_move(ml, 7, 6));     // h1xg1 captures the checking rook
  EXPECT_FALSE(has_move(ml, 7, 15));   // h1h2 does not escape check
  EXPECT_EQ(ml.size(), 4);             // 3 king + Rg1
}

TEST(MoveGen, CastlingOutOfCheckIllegal) {
  // Black rook e8 checks the white king e1 down the e-file.
  Board b = board_from("k3r3/8/8/8/8/8/8/4K2R w K - 0 1");
  EXPECT_TRUE(in_check(b));
  const MoveList ml = generate_moves(b);
  EXPECT_FALSE(has_move(ml, 4, 6));
  EXPECT_EQ(ml.size(), 4);  // 4 king moves (d1, d2, f1, f2)
}

TEST(MoveGen, PromotionMoveGeneration) {
  Board b = board_from("8/P7/8/8/8/8/8/4K2k w - - 0 1");
  const MoveList ml = generate_moves(b);
  EXPECT_TRUE(has_move(ml, 48, 56, PieceType::Queen));
  EXPECT_TRUE(has_move(ml, 48, 56, PieceType::Knight));
  EXPECT_FALSE(has_move(ml, 48, 56));  // no unpromoted a7a8
  EXPECT_EQ(ml.size(), 9);             // 4 promotions + 5 king moves
}

TEST(MoveGen, KingCannotMoveIntoCheck) {
  Board b = board_from("4r3/8/8/8/8/8/8/4K2k w - - 0 1");
  const MoveList ml = generate_moves(b);
  EXPECT_FALSE(has_move(ml, 4, 12));  // e1e2 into the e-file
  EXPECT_TRUE(has_move(ml, 4, 13));   // e1f2 safe
  EXPECT_EQ(ml.size(), 4);
}

TEST(MoveGen, PinnedPieceCannotExposeCheck) {
  // White rook a2 is pinned to king a1 by black rook a8.
  Board b = board_from("r3k3/8/8/8/8/8/R7/K7 w - - 0 1");
  const MoveList ml = generate_moves(b);
  EXPECT_FALSE(has_move(ml, 8, 9));  // a2b2 exposes the king
  EXPECT_TRUE(has_move(ml, 8, 56));   // a2xa8 captures the checking rook
  EXPECT_EQ(ml.size(), 8);            // 6 rook + 2 king
}

TEST(MoveGen, PerftStartPosition) {
  Board b = board_from("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  EXPECT_EQ(perft(b, 1), 20);
  EXPECT_EQ(perft(b, 2), 400);
  EXPECT_EQ(perft(b, 3), 8902);
  EXPECT_EQ(perft(b, 4), 197281);
  EXPECT_EQ(perft(b, 5), 4865609);
}

TEST(MoveGen, PerftKiwiPete) {
  Board b = board_from("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  EXPECT_EQ(perft(b, 1), 48);
  EXPECT_EQ(perft(b, 2), 2039);
  EXPECT_EQ(perft(b, 3), 97862);
  EXPECT_EQ(perft(b, 4), 4085603);
}

TEST(MoveGen, PerftPosition3) {
  Board b = board_from("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
  EXPECT_EQ(perft(b, 1), 14);
  EXPECT_EQ(perft(b, 2), 191);
  EXPECT_EQ(perft(b, 3), 2812);
}

TEST(MoveGen, PerftPosition4) {
  Board b = board_from("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
  EXPECT_EQ(perft(b, 1), 6);
  EXPECT_EQ(perft(b, 2), 264);
  EXPECT_EQ(perft(b, 3), 9467);
  EXPECT_EQ(perft(b, 4), 422333);
}

TEST(MoveGen, PerftPosition5) {
  Board b = board_from("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
  EXPECT_EQ(perft(b, 1), 44);
  EXPECT_EQ(perft(b, 2), 1486);
  EXPECT_EQ(perft(b, 3), 62379);
}
