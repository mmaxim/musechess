#include "eval.h"
#include "movegen/bitboard.h"

namespace chess {

namespace {
// Material values in centipawns, based on common chess programming references.
// Piece-square tables adapted from Chess Programming Wiki Simplified Evaluation Function
// and typical CPW PSTs (see https://www.chessprogramming.org/Simplified_Evaluation_Function).
constexpr int kMaterial[12] = {
    100, 320, 330, 500, 900, 20000,
    100, 320, 330, 500, 900, 20000
};

// Piece-square tables for white, in centipawns.
constexpr int kPawnTable[64] = {
  0,  0,  0,  0,  0,  0,  0,  0,
 10, 10, 10, 10, 10, 10, 10, 10,
 20, 20, 20, 20, 20, 20, 20, 20,
 30, 30, 30, 30, 30, 30, 30, 30,
 40, 40, 40, 40, 40, 40, 40, 40,
 50, 50, 50, 50, 50, 50, 50, 50,
 60, 60, 60, 60, 60, 60, 60, 60,
 80, 80, 80, 80, 80, 80, 80, 80
};

constexpr int kKnightTable[64] = {
 -50,-40,-30,-30,-30,-30,-40,-50,
 -40,-20,  0,  0,  0,  0,-20,-40,
 -30,  0, 10, 15, 15, 10,  0,-30,
 -30,  5, 15, 20, 20, 15,  5,-30,
 -30,  0, 15, 20, 20, 15,  0,-30,
 -30,  5, 10, 15, 15, 10,  5,-30,
 -40,-20,  0,  5,  5,  0,-20,-40,
 -50,-40,-30,-30,-30,-30,-40,-50
};

constexpr int kBishopTable[64] = {
 -20,-10,-10,-10,-10,-10,-10,-20,
 -10,  0,  0,  0,  0,  0,  0,-10,
 -10,  0,  5,  5,  5,  5,  0,-10,
 -10,  0,  5,  5,  5,  5,  0,-10,
 -10,  0,  5,  5,  5,  5,  0,-10,
 -10,  0,  5,  5,  5,  5,  0,-10,
 -10,  0,  0,  0,  0,  0,  0,-10,
 -20,-10,-10,-10,-10,-10,-10,-20
};

constexpr int kRookTable[64] = {
  0,  0,  0,  0,  0,  0,  0,  0,
  5, 10, 10, 10, 10, 10, 10,  5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
  5, 10, 10, 10, 10, 10, 10,  5,
  0,  0,  0,  0,  0,  0,  0,  0
};

constexpr int kQueenTable[64] = {
 -20,-10,-10, -5, -5,-10,-10,-20,
 -10,  0,  0,  0,  0,  0,  0,-10,
 -10,  0,  5,  5,  5,  5,  0,-10,
  -5,  0,  5,  5,  5,  5,  0, -5,
  -5,  0,  5,  5,  5,  5,  0, -5,
 -10,  0,  5,  5,  5,  5,  0,-10,
 -10,  0,  0,  0,  0,  0,  0,-10,
 -20,-10,-10, -5, -5,-10,-10,-20
};

constexpr int kKingTable[64] = {
 -30,-40,-40,-50,-50,-40,-40,-30,
 -30,-40,-40,-50,-50,-40,-40,-30,
 -30,-40,-40,-50,-50,-40,-40,-30,
 -30,-40,-40,-50,-50,-40,-40,-30,
 -20,-30,-30,-40,-40,-30,-30,-20,
 -10,-20,-20,-20,-20,-20,-20,-10,
  20, 20,  0,  0,  0,  0, 20, 20,
  20, 30, 10,  0,  0, 10, 30, 20
};

inline int pst(PieceType t, int sq, Color c) {
  using namespace bitboard;
  int r = rank_of(sq);
  if (c == Color::Black) r = 7 - r;
  int idx = r * 8 + file_of(sq);
  switch (t) {
    case PieceType::Pawn:   return kPawnTable[idx];
    case PieceType::Knight: return kKnightTable[idx];
    case PieceType::Bishop: return kBishopTable[idx];
    case PieceType::Rook:   return kRookTable[idx];
    case PieceType::Queen:  return kQueenTable[idx];
    case PieceType::King:   return kKingTable[idx];
    default: return 0;
  }
}
}

int MaterialEvaluator::evaluate(const Board& b) const {
  auto score = 0;
  for (auto i = 0; i < kNumPieces; ++i) {
    auto piece = static_cast<Piece>(i);
    auto pt = type_of_piece(piece);
    auto c = color_of_piece(piece);
    auto mask = b.masks[i];
    while (mask) {
      auto sq = bitboard::ctz(mask);
      mask &= mask - 1;
      auto material = kMaterial[i];
      auto positional = pst(pt, static_cast<int>(sq), c);
      auto val = material + positional;
      if (c == Color::White) {
        score += val;
      } else {
        score -= val;
      }
    }
  }
  return b.side_to_move == Color::White ? score : -score;
}

}  // namespace chess
