#pragma once

#include <cstdint>
#include <string>

#include "bitboard.h"

namespace chess {
class Board;

enum class PieceType : std::uint8_t {
  Pawn,
  Knight,
  Bishop,
  Rook,
  Queen,
  King,
  None,
};

inline constexpr char piece_char(PieceType t) {
  switch (t) {
    case PieceType::Pawn: return 'P';
    case PieceType::Knight: return 'N';
    case PieceType::Bishop: return 'B';
    case PieceType::Rook: return 'R';
    case PieceType::Queen: return 'Q';
    case PieceType::King: return 'K';
    default: return '?';
  }
}

struct Move {
  int from = -1;
  int to = -1;
  PieceType promotion = PieceType::None;
  std::uint8_t flags = 0;

  static constexpr std::uint8_t kDoublePawn = 1u << 0;
  static constexpr std::uint8_t kCastling = 1u << 1;
  static constexpr std::uint8_t kEnPassant = 1u << 2;

  bool has_flag(std::uint8_t f) const { return (flags & f) != 0; }

  bool operator==(const Move& o) const {
    return from == o.from && to == o.to && promotion == o.promotion && flags == o.flags;
  }
  bool operator!=(const Move& o) const { return !(*this == o); }

  // "e2e4", "e7e8=q", "e1g1", ... (no check/capture suffixes)
  std::string to_string() const {
    const auto sq_name = [](int sq) {
      return std::string(1, static_cast<char>('a' + bitboard::file_of(sq))) +
             std::string(1, static_cast<char>('1' + bitboard::rank_of(sq)));
    };
    std::string s = sq_name(from) + sq_name(to);
    if (promotion != PieceType::None) {
      s += '=';
      s += piece_char(promotion);
    }
    return s;
  }

  // Standard Algebraic Notation, requires board context
  std::string to_san(const Board& board) const;
};

class MoveList {
 public:
  static constexpr int kMax = 256;

  void add(Move m) {
    if (count_ >= kMax) return;
    moves_[count_++] = m;
  }
  int size() const { return count_; }
  bool empty() const { return count_ == 0; }
  Move& operator[](int i) { return moves_[i]; }
  const Move& operator[](int i) const { return moves_[i]; }
  Move* begin() { return moves_; }
  const Move* begin() const { return moves_; }
  Move* end() { return moves_ + count_; }
  const Move* end() const { return moves_ + count_; }

 private:
  Move moves_[kMax];
  int count_ = 0;
};

}  // namespace chess
