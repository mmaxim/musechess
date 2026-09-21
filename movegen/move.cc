#include "move.h"
#include "board.h"
#include "movegen.h"

namespace chess {

static std::string sq_name(int sq) {
  return std::string(1, static_cast<char>('a' + bitboard::file_of(sq))) +
         std::string(1, static_cast<char>('1' + bitboard::rank_of(sq)));
}

std::string Move::to_san(const Board& board) const {
  if (from < 0 || to < 0) return to_string();
  Piece moving_piece = board.piece_at(from);
  if (moving_piece == kNumPieces) return to_string();

  Color me = color_of_piece(moving_piece);
  PieceType pt = type_of_piece(moving_piece);

  bool is_capture = false;
  if (has_flag(kEnPassant)) {
    is_capture = true;
  } else {
    Piece target = board.piece_at(to);
    if (target != kNumPieces && color_of_piece(target) == opponent(me)) {
      is_capture = true;
    }
  }

  bool is_castling = has_flag(kCastling);
  bool is_promotion = promotion != PieceType::None;

  std::string s;
  if (is_castling) {
    // King side: e1-g1 / e8-g8, Queen side: e1-c1 / e8-c8
    if (to == 6 || to == 62) s = "O-O";
    else if (to == 2 || to == 58) s = "O-O-O";
    else s = sq_name(from) + sq_name(to);
  } else if (pt == PieceType::Pawn) {
    if (is_capture) {
      s += static_cast<char>('a' + bitboard::file_of(from));
      s += 'x';
    }
    s += sq_name(to);
    if (is_promotion) {
      s += '=';
      s += piece_char(promotion);
    }
  } else {
    s += piece_char(pt);
    {
      auto moves = generate_moves(board);
      int count = 0;
      bool same_file = true;
      int common_file = bitboard::file_of(from);
      int common_rank = bitboard::rank_of(from);
      for (const auto& m : moves) {
        if (m.to == to && type_of_piece(board.piece_at(m.from)) == pt) {
          count++;
          if (bitboard::file_of(m.from) != common_file) same_file = false;
          if (bitboard::rank_of(m.from) != common_rank) common_rank = -1;
        }
      }
      if (count > 1) {
        if (same_file) {
          s += static_cast<char>('1' + bitboard::rank_of(from));
        } else {
          s += static_cast<char>('a' + bitboard::file_of(from));
        }
      }
    }
    if (is_capture) s += 'x';
    s += sq_name(to);
    if (is_promotion) {
      s += '=';
      s += piece_char(promotion);
    }
  }

  // Check / checkmate
  {
    Board tmp = board;
    tmp.make_move(*this);
    if (in_check(tmp)) {
      // Could check for mate by generating moves, but keep simple
      s += '+';
    }
  }

  return s;
}

}  // namespace chess
