#pragma once

#include <cstdint>
#include <string>

#include "bitboard.h"
#include "move.h"

namespace chess {

enum class Color : std::uint8_t { White, Black };

inline constexpr Color opponent(Color c) {
  return c == Color::White ? Color::Black : Color::White;
}

// 12 piece masks, one per (color, piece type) pair.
enum Piece : std::uint8_t {
  kWhitePawn = 0,
  kWhiteKnight,
  kWhiteBishop,
  kWhiteRook,
  kWhiteQueen,
  kWhiteKing,
  kBlackPawn,
  kBlackKnight,
  kBlackBishop,
  kBlackRook,
  kBlackQueen,
  kBlackKing,
  kNumPieces,
};

inline constexpr Piece piece_index(Color c, PieceType t) {
  return static_cast<Piece>(static_cast<int>(c) * 6 + static_cast<int>(t));
}
inline constexpr Color color_of_piece(Piece p) {
  return p < kBlackPawn ? Color::White : Color::Black;
}
inline constexpr PieceType type_of_piece(Piece p) {
  return static_cast<PieceType>(p % 6);
}

struct Board {
  U64 masks[kNumPieces] = {};
  Color side_to_move = Color::White;
  std::uint8_t castling = 0;
  int ep_square = -1;  // Square an en-passant capture would land on, or -1.
  int halfmove_clock = 0;
  int fullmove = 1;

  // Castling right bits.
  static constexpr std::uint8_t kWhiteKingside = 1u << 0;
  static constexpr std::uint8_t kWhiteQueenside = 1u << 1;
  static constexpr std::uint8_t kBlackKingside = 1u << 2;
  static constexpr std::uint8_t kBlackQueenside = 1u << 3;

  U64 occupancy() const;
  U64 color_mask(Color c) const;
  U64 type_mask(PieceType t) const;

  // Returns kNumPieces if the square is empty.
  Piece piece_at(int sq) const;

  // Parses a standard FEN (6 fields). Returns false on malformed input and
  // leaves the board unchanged.
  bool set_fen(const std::string& fen);
  std::string to_fen() const;

  // Applies a move in place (mutates this board). Assumes the move is legal
  // with respect to this board's side to move.
  void make_move(const Move& m);
};

}  // namespace chess
