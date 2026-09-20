#include <gtest/gtest.h>

#include <cstdint>

#include "board.h"
#include "movegen.h"

using namespace chess;

namespace {

constexpr int sq(int rank, int file) { return rank * 8 + file; }

const char* const kStart =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

Board board_from(const char* fen) {
  Board b;
  EXPECT_TRUE(b.set_fen(fen));
  return b;
}

Move mv(int from, int to, PieceType promo = PieceType::None, std::uint8_t flags = 0) {
  Move m;
  m.from = from;
  m.to = to;
  m.promotion = promo;
  m.flags = flags;
  return m;
}

}  // namespace

TEST(Board, FenRoundTripStart) {
  Board b = board_from(kStart);
  EXPECT_EQ(b.to_fen(), kStart);
}

TEST(Board, FenRoundTripKiwiPete) {
  const char* fen =
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
  Board b = board_from(fen);
  EXPECT_EQ(b.to_fen(), fen);
}

TEST(Board, FenParsingRejectsBadInput) {
  Board b;
  EXPECT_FALSE(b.set_fen(""));
  EXPECT_FALSE(b.set_fen("not a fen at all"));
  EXPECT_FALSE(b.set_fen("k/8/8/8/8/8/8/K v - - 0 1"));  // invalid side to move
  EXPECT_FALSE(b.set_fen("K/K/8/8/8/8/8/8 w KQkq - 0 1"));  // two white kings, no black king
  EXPECT_FALSE(b.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -x 0 1"));
}

TEST(Board, OccupancyAndPieceAt) {
  Board b = board_from(kStart);
  EXPECT_EQ(bitboard::popcount(b.occupancy()), 32);
  EXPECT_EQ(b.piece_at(4), kWhiteKing);   // e1
  EXPECT_EQ(b.piece_at(56), kBlackRook);  // a8
  EXPECT_EQ(b.piece_at(0), kWhiteRook);   // a1
  EXPECT_EQ(b.piece_at(27), kNumPieces);  // d4 empty
}

TEST(Board, MakeMovePawnPush) {
  Board b = board_from(kStart);
  b.make_move(mv(12, 28));  // e2e4
  EXPECT_EQ(b.to_fen(), "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
}

TEST(Board, MakeMoveDoublePushSetsEpSquare) {
  Board b = board_from(kStart);
  b.make_move(mv(11, 27));  // d2d4
  b.make_move(mv(51, 35));  // d7d5
  EXPECT_EQ(b.ep_square, sq(5, 3));  // d6
  EXPECT_EQ(b.to_fen(), "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq d6 0 2");
}

TEST(Board, MakeMoveKingLosesCastlingRights) {
  Board b = board_from(kStart);
  b.make_move(mv(4, 6));  // e1g1 (state-update check, not a legal move)
  EXPECT_EQ(b.castling, (Board::kBlackKingside | Board::kBlackQueenside));
}

TEST(Board, MakeMoveRookCaptureRemovesRights) {
  Board b = board_from("r3k3/8/8/8/8/8/8/R3K3 w q - 0 1");
  b.make_move(mv(0, 56));  // a1xa8 captures black's queenside rook
  EXPECT_EQ(b.castling, 0);  // black loses its queenside right
}

TEST(Board, MakeMoveEnPassantRemovesCapturedPawn) {
  Board b = board_from("7k/8/8/3Pp3/8/8/8/K7 w - e6 0 1");
  b.make_move(mv(35, 44, PieceType::None, Move::kEnPassant));  // d5xe6 e.p.
  EXPECT_EQ(b.piece_at(36), kNumPieces);  // black pawn on e5 gone
  EXPECT_EQ(b.piece_at(44), kWhitePawn);  // white pawn now on e6
  EXPECT_EQ(b.to_fen(), "7k/8/4P3/8/8/8/8/K7 b - - 0 1");
}

TEST(Board, MakeMovePromotion) {
  Board b = board_from("8/P7/8/8/8/8/8/4K2k w - - 0 1");
  b.make_move(mv(48, 56, PieceType::Queen));  // a7a8=Q
  EXPECT_EQ(b.piece_at(56), kWhiteQueen);
  EXPECT_EQ(b.piece_at(48), kNumPieces);
}

TEST(Board, MakeMoveCastlingMovesRook) {
  Board b = board_from("7k/8/8/8/8/8/8/4K2R w K - 0 1");
  b.make_move(mv(4, 6, PieceType::None, Move::kCastling));  // e1g1
  EXPECT_EQ(b.piece_at(6), kWhiteKing);   // king on g1
  EXPECT_EQ(b.piece_at(5), kWhiteRook);   // rook on f1
  EXPECT_EQ(b.piece_at(7), kNumPieces);   // rook left h1
  // Queenside
  Board c = board_from("7k/8/8/8/8/8/8/R3K3 w Q - 0 1");
  c.make_move(mv(4, 2, PieceType::None, Move::kCastling));  // e1c1
  EXPECT_EQ(c.piece_at(2), kWhiteKing);   // king on c1
  EXPECT_EQ(c.piece_at(3), kWhiteRook);   // rook on d1
  EXPECT_EQ(c.piece_at(0), kNumPieces);   // rook left a1
}

TEST(Board, InCheckDetection) {
  // Start position: not in check.
  Board b0 = board_from(kStart);
  EXPECT_FALSE(in_check(b0));

  // White king a1 in check by black rook a8 (open a-file).
  Board b1 = board_from("r3k3/8/8/8/8/8/8/K7 w - - 0 1");
  EXPECT_TRUE(in_check(b1));

  // Same, but white pawn a2 blocks the a-file: not in check.
  Board b2 = board_from("r3k3/8/8/8/8/8/P7/K7 w - - 0 1");
  EXPECT_FALSE(in_check(b2));

  // Black king e8 in check by white queen d7 (diagonal).
  Board b3 = board_from("4k3/3Q4/8/8/8/8/8/K7 b - - 0 1");
  EXPECT_TRUE(in_check(b3));
}
