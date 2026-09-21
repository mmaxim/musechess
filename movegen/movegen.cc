#include "movegen.h"

#include <array>

#include "bitboard.h"

namespace chess {
namespace {

using namespace bitboard;

struct Dir {
  int dr, df;
};

constexpr std::array<Dir, 8> kKnightDirs = {{
    {1, 2}, {1, -2}, {2, 1}, {2, -1}, {-1, 2}, {-1, -2}, {-2, 1}, {-2, -1},
}};
constexpr std::array<Dir, 8> kKingDirs = {{
    {1, 1}, {1, 0}, {1, -1}, {0, 1}, {0, -1}, {-1, 1}, {-1, 0}, {-1, -1},
}};
constexpr std::array<Dir, 4> kBishopDirs = {{
    {1, 1}, {1, -1}, {-1, 1}, {-1, -1},
}};
constexpr std::array<Dir, 4> kRookDirs = {{
    {1, 0}, {-1, 0}, {0, 1}, {0, -1},
}};
constexpr std::array<Dir, 8> kQueenDirs = {{
    {1, 1}, {1, 0}, {1, -1}, {0, 1}, {0, -1}, {-1, 1}, {-1, 0}, {-1, -1},
}};

// All squares a set of knights can reach (within the board).
U64 knight_attack_mask(U64 knights) {
  auto out = U64{0};
  while (knights) {
    auto sq = ctz(knights);
    knights &= knights - 1;
    for (const Dir& d : kKnightDirs) {
      auto r = rank_of(sq) + d.dr;
      auto f = file_of(sq) + d.df;
      if (r < 0 || r > 7 || f < 0 || f > 7) continue;
      out |= set_bit(square(r, f));
    }
  }
  return out;
}

// All squares a set of kings can reach (within the board).
U64 king_attack_mask(U64 kings) {
  auto out = U64{0};
  while (kings) {
    auto sq = ctz(kings);
    kings &= kings - 1;
    for (const Dir& d : kKingDirs) {
      auto r = rank_of(sq) + d.dr;
      auto f = file_of(sq) + d.df;
      if (r < 0 || r > 7 || f < 0 || f > 7) continue;
      out |= set_bit(square(r, f));
    }
  }
  return out;
}

// Sliding attack rays: every square along each direction up to and including
// the first occupied square.
U64 ray_attack_mask(U64 sliders, U64 occ, const std::array<Dir, 4>& dirs) {
  auto out = U64{0};
  while (sliders) {
    auto sq = ctz(sliders);
    sliders &= sliders - 1;
    for (const Dir& d : dirs) {
      auto cur = set_bit(sq);
      while (true) {
        auto nxt = shift_dir(cur, d.dr, d.df);
        if (!nxt) break;
        out |= nxt;
        if (nxt & occ) break;
        cur = nxt;
      }
    }
  }
  return out;
}

U64 queen_attack_mask(U64 queens, U64 occ) {
  return ray_attack_mask(queens, occ, kBishopDirs) | ray_attack_mask(queens, occ, kRookDirs);
}

constexpr std::array<PieceType, 4> kPromotions = {
    PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight,
};

}  // namespace

U64 attacked_by(const Board& b, Color c) {
  auto occ = b.occupancy();
  auto pawns = b.masks[piece_index(c, PieceType::Pawn)];
  auto out = U64{};
  if (c == Color::White) {
    out = shift_dir(pawns, 1, 1) | shift_dir(pawns, 1, -1);
  } else {
    out = shift_dir(pawns, -1, 1) | shift_dir(pawns, -1, -1);
  }
  out |= knight_attack_mask(b.masks[piece_index(c, PieceType::Knight)]);
  out |= king_attack_mask(b.masks[piece_index(c, PieceType::King)]);
  out |= ray_attack_mask(b.masks[piece_index(c, PieceType::Bishop)], occ, kBishopDirs);
  out |= ray_attack_mask(b.masks[piece_index(c, PieceType::Rook)], occ, kRookDirs);
  out |= queen_attack_mask(b.masks[piece_index(c, PieceType::Queen)], occ);
  return out;
}

bool in_check(const Board& b) {
  const auto king = b.masks[piece_index(b.side_to_move, PieceType::King)];
  return (attacked_by(b, opponent(b.side_to_move)) & king) != 0;
}

namespace {

void gen_pawn_moves(const Board& b, MoveList& out) {
  auto me = b.side_to_move;
  auto opp = opponent(me);
  auto occ = b.occupancy();
  auto pawns = b.masks[piece_index(me, PieceType::Pawn)];
  auto enemies = b.color_mask(opp);
  auto white = (me == Color::White);
  auto promo_rank = white ? 7 : 0;

  auto add_promotions = [&](Move m) {
    for (PieceType pt : kPromotions) {
      Move p = m;
      p.promotion = pt;
      out.add(p);
    }
  };
  auto emit = [&](Move m) {
    if (rank_of(m.to) == promo_rank) add_promotions(m);
    else out.add(m);
  };

  if (white) {
    // Single pushes.
    auto pushes = (pawns << 8) & ~occ;
    while (pushes) {
      auto d = ctz(pushes);
      pushes &= pushes - 1;
      Move m;
      m.from = d - 8;
      m.to = d;
      emit(m);
    }
    // Double pushes.
    auto doubles = (pawns & rank_mask(1)) << 16 & ~occ;
    while (doubles) {
      auto d = ctz(doubles);
      doubles &= doubles - 1;
      auto origin = d - 16;
      if (test_bit(occ, origin + 8)) continue;  // Path blocked.
      Move m;
      m.from = origin;
      m.to = d;
      m.flags = Move::kDoublePawn;
      out.add(m);
    }
    // Captures.
    {
      auto pawn_bits = pawns;
      while (pawn_bits) {
        auto sq = ctz(pawn_bits);
        pawn_bits &= pawn_bits - 1;
        auto left = shift_dir(set_bit(sq), 1, -1) & enemies;
        while (left) {
          auto d = ctz(left);
          left &= left - 1;
          Move m;
          m.from = sq;
          m.to = d;
          emit(m);
        }
        auto right = shift_dir(set_bit(sq), 1, 1) & enemies;
        while (right) {
          auto d = ctz(right);
          right &= right - 1;
          Move m;
          m.from = sq;
          m.to = d;
          emit(m);
        }
      }
    }
    // En-passant.
    if (b.ep_square >= 0) {
      const auto ep_mask = set_bit(b.ep_square);
      auto capturers = pawns & (shift_dir(ep_mask, -1, 1) | shift_dir(ep_mask, -1, -1));
      while (capturers) {
        auto o = ctz(capturers);
        capturers &= capturers - 1;
        Move m;
        m.from = o;
        m.to = b.ep_square;
        m.flags = Move::kEnPassant;
        out.add(m);
      }
    }
  } else {
    // Single pushes.
    auto pushes = (pawns >> 8) & ~occ;
    while (pushes) {
      auto d = ctz(pushes);
      pushes &= pushes - 1;
      Move m;
      m.from = d + 8;
      m.to = d;
      emit(m);
    }
    // Double pushes.
    auto doubles = (pawns & rank_mask(6)) >> 16 & ~occ;
    while (doubles) {
      auto d = ctz(doubles);
      doubles &= doubles - 1;
      auto origin = d + 16;
      if (test_bit(occ, origin - 8)) continue;  // Path blocked.
      Move m;
      m.from = origin;
      m.to = d;
      m.flags = Move::kDoublePawn;
      out.add(m);
    }
    // Captures.
    {
      auto pawn_bits = pawns;
      while (pawn_bits) {
        auto sq = ctz(pawn_bits);
        pawn_bits &= pawn_bits - 1;
        auto left = shift_dir(set_bit(sq), -1, -1) & enemies;
        while (left) {
          auto d = ctz(left);
          left &= left - 1;
          Move m;
          m.from = sq;
          m.to = d;
          emit(m);
        }
        auto right = shift_dir(set_bit(sq), -1, 1) & enemies;
        while (right) {
          auto d = ctz(right);
          right &= right - 1;
          Move m;
          m.from = sq;
          m.to = d;
          emit(m);
        }
      }
    }
    // En-passant.
    if (b.ep_square >= 0) {
      const auto ep_mask = set_bit(b.ep_square);
      auto capturers = pawns & (shift_dir(ep_mask, 1, 1) | shift_dir(ep_mask, 1, -1));
      while (capturers) {
        auto o = ctz(capturers);
        capturers &= capturers - 1;
        Move m;
        m.from = o;
        m.to = b.ep_square;
        m.flags = Move::kEnPassant;
        out.add(m);
      }
    }
  }
}

void gen_knight_moves(const Board& b, MoveList& out) {
  auto knights = b.masks[piece_index(b.side_to_move, PieceType::Knight)];
  const auto own = b.color_mask(b.side_to_move);
  while (knights) {
    auto sq = ctz(knights);
    knights &= knights - 1;
    auto dests = knight_attack_mask(set_bit(sq));
    while (dests) {
      auto d = ctz(dests);
      dests &= dests - 1;
      if (test_bit(own, d)) continue;
      Move m;
      m.from = sq;
      m.to = d;
      out.add(m);
    }
  }
}

void gen_king_moves(const Board& b, MoveList& out) {
  auto kings = b.masks[piece_index(b.side_to_move, PieceType::King)];
  const auto own = b.color_mask(b.side_to_move);
  while (kings) {
    auto sq = ctz(kings);
    kings &= kings - 1;
    auto dests = king_attack_mask(set_bit(sq));
    while (dests) {
      auto d = ctz(dests);
      dests &= dests - 1;
      if (test_bit(own, d)) continue;
      Move m;
      m.from = sq;
      m.to = d;
      out.add(m);
    }
  }
}

void gen_bishop_moves(const Board& b, MoveList& out) {
  auto bishops = b.masks[piece_index(b.side_to_move, PieceType::Bishop)];
  const auto occ = b.occupancy();
  const auto own = b.color_mask(b.side_to_move);
  while (bishops) {
    auto sq = ctz(bishops);
    bishops &= bishops - 1;
    auto dests = ray_attack_mask(set_bit(sq), occ, kBishopDirs);
    while (dests) {
      auto d = ctz(dests);
      dests &= dests - 1;
      if (test_bit(own, d)) continue;
      Move m;
      m.from = sq;
      m.to = d;
      out.add(m);
    }
  }
}

void gen_rook_moves(const Board& b, MoveList& out) {
  auto rooks = b.masks[piece_index(b.side_to_move, PieceType::Rook)];
  const auto occ = b.occupancy();
  const auto own = b.color_mask(b.side_to_move);
  while (rooks) {
    auto sq = ctz(rooks);
    rooks &= rooks - 1;
    auto dests = ray_attack_mask(set_bit(sq), occ, kRookDirs);
    while (dests) {
      auto d = ctz(dests);
      dests &= dests - 1;
      if (test_bit(own, d)) continue;
      Move m;
      m.from = sq;
      m.to = d;
      out.add(m);
    }
  }
}

void gen_queen_moves(const Board& b, MoveList& out) {
  auto queens = b.masks[piece_index(b.side_to_move, PieceType::Queen)];
  const auto occ = b.occupancy();
  const auto own = b.color_mask(b.side_to_move);
  while (queens) {
    auto sq = ctz(queens);
    queens &= queens - 1;
    auto dests = queen_attack_mask(set_bit(sq), occ);
    while (dests) {
      auto d = ctz(dests);
      dests &= dests - 1;
      if (test_bit(own, d)) continue;
      Move m;
      m.from = sq;
      m.to = d;
      out.add(m);
    }
  }
}

void gen_castling(const Board& b, MoveList& out) {
  auto me = b.side_to_move;
  auto opp = opponent(me);
  if (me == Color::White) {
    if ((b.castling & (Board::kWhiteKingside | Board::kWhiteQueenside)) == 0) return;
    if (!(b.masks[kWhiteKing] & set_bit(4))) return;  // King not on e1.
    const auto occ = b.occupancy();
    const auto attacked = attacked_by(b, opp);
    if (test_bit(attacked, 4)) return;  // Cannot castle out of check.
    if ((b.castling & Board::kWhiteKingside) &&
        !test_bit(occ, 5) && !test_bit(occ, 6) &&
        !test_bit(attacked, 5) && !test_bit(attacked, 6)) {
      Move m;
      m.from = 4;
      m.to = 6;
      m.flags = Move::kCastling;
      out.add(m);
    }
    if ((b.castling & Board::kWhiteQueenside) &&
        !test_bit(occ, 1) && !test_bit(occ, 2) && !test_bit(occ, 3) &&
        !test_bit(attacked, 2) && !test_bit(attacked, 3)) {
      Move m;
      m.from = 4;
      m.to = 2;
      m.flags = Move::kCastling;
      out.add(m);
    }
  } else {
    if ((b.castling & (Board::kBlackKingside | Board::kBlackQueenside)) == 0) return;
    if (!(b.masks[kBlackKing] & set_bit(60))) return;  // King not on e8.
    const auto occ = b.occupancy();
    const auto attacked = attacked_by(b, opp);
    if (test_bit(attacked, 60)) return;  // Cannot castle out of check.
    if ((b.castling & Board::kBlackKingside) &&
        !test_bit(occ, 61) && !test_bit(occ, 62) &&
        !test_bit(attacked, 61) && !test_bit(attacked, 62)) {
      Move m;
      m.from = 60;
      m.to = 62;
      m.flags = Move::kCastling;
      out.add(m);
    }
    if ((b.castling & Board::kBlackQueenside) &&
        !test_bit(occ, 57) && !test_bit(occ, 58) && !test_bit(occ, 59) &&
        !test_bit(attacked, 58) && !test_bit(attacked, 59)) {
      Move m;
      m.from = 60;
      m.to = 58;
      m.flags = Move::kCastling;
      out.add(m);
    }
  }
}

}  // namespace

MoveList generate_pseudo_legal(const Board& b) {
  MoveList out;
  gen_pawn_moves(b, out);
  gen_knight_moves(b, out);
  gen_bishop_moves(b, out);
  gen_rook_moves(b, out);
  gen_queen_moves(b, out);
  gen_king_moves(b, out);
  gen_castling(b, out);
  return out;
}

MoveList generate_moves(const Board& b) {
  MoveList legal;
  const MoveList pseudo = generate_pseudo_legal(b);
  auto me = b.side_to_move;
  for (const Move& m : pseudo) {
    Board after = b;
    after.make_move(m);
    // Legal iff the mover's own king is no longer attacked.
    const auto my_king_after = after.masks[piece_index(me, PieceType::King)];
    if ((attacked_by(after, opponent(me)) & my_king_after) == 0) legal.add(m);
  }
  return legal;
}

}  // namespace chess
