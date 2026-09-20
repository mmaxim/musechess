#include "board.h"

#include <cctype>
#include <cstdlib>
#include <vector>

#include "bitboard.h"

namespace chess {

namespace {
constexpr int kA1 = 0, kH1 = 7;
constexpr int kA8 = 56, kH8 = 63;
constexpr int kD1 = 3, kF1 = 5;
constexpr int kD8 = 59, kF8 = 61;

char to_lower(char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }

std::uint8_t corner_rights(int sq) {
  switch (sq) {
    case kA1: return Board::kWhiteQueenside;
    case kH1: return Board::kWhiteKingside;
    case kA8: return Board::kBlackQueenside;
    case kH8: return Board::kBlackKingside;
    default: return 0;
  }
}
}  // namespace

U64 Board::occupancy() const {
  auto occ = U64{0};
  for (auto i = 0; i < kNumPieces; ++i) occ |= masks[i];
  return occ;
}

U64 Board::color_mask(Color c) const {
  auto m = U64{0};
  for (auto i = 0; i < kNumPieces; ++i) {
    if (color_of_piece(static_cast<Piece>(i)) == c) m |= masks[i];
  }
  return m;
}

U64 Board::type_mask(PieceType t) const {
  return masks[piece_index(Color::White, t)] | masks[piece_index(Color::Black, t)];
}

Piece Board::piece_at(int sq) const {
  for (auto i = 0; i < kNumPieces; ++i) {
    if (bitboard::test_bit(masks[i], sq)) return static_cast<Piece>(i);
  }
  return kNumPieces;
}

bool Board::set_fen(const std::string& fen) {
  std::vector<std::string> fields;
  {
    auto current = std::string{};
    for (auto c : fen) {
      if (c == ' ') {
        if (!current.empty()) { fields.push_back(current); current.clear(); }
      } else {
        current.push_back(c);
      }
    }
    if (!current.empty()) fields.push_back(current);
  }
  // Tolerate missing trailing fields (side, castling, ep, clocks).
  if (fields.empty() || fields.size() > 6) return false;

  Board b;

  // 1) Piece placement.
  {
    auto &placement = fields[0];
    auto rank = 7;
    auto file = 0;
    for (auto c : placement) {
      if (c == '/') {
        if (rank == 0) return false;  // Trailing '/' or too many ranks.
        --rank;
        file = 0;
        continue;
      }
      if (file >= 8) return false;
      if (std::isdigit(static_cast<unsigned char>(c))) {
        file += c - '0';
        if (file > 8) return false;
        continue;
      }
      auto pc = to_lower(c);
      PieceType t;
      switch (pc) {
        case 'p': t = PieceType::Pawn;   break;
        case 'n': t = PieceType::Knight; break;
        case 'b': t = PieceType::Bishop; break;
        case 'r': t = PieceType::Rook;   break;
        case 'q': t = PieceType::Queen;  break;
        case 'k': t = PieceType::King;   break;
        default: return false;
      }
      Color col = (c >= 'A' && c <= 'Z') ? Color::White : Color::Black;
      b.masks[piece_index(col, t)] |= bitboard::set_bit(bitboard::square(rank, file));
      ++file;
    }
    if (rank != 0 || file != 8) return false;  // Placement didn't end exactly at a8.
    if (bitboard::popcount(b.masks[kWhiteKing]) != 1) return false;
    if (bitboard::popcount(b.masks[kBlackKing]) != 1) return false;
  }

  // 2) Side to move.
  if (fields.size() >= 2) {
    if (fields[1] == "w") b.side_to_move = Color::White;
    else if (fields[1] == "b") b.side_to_move = Color::Black;
    else return false;
  }

  // 3) Castling rights.
  if (fields.size() >= 3) {
    auto &c = fields[2];
    if (c != "-") {
      for (auto ch : c) {
        switch (ch) {
          case 'K': b.castling |= Board::kWhiteKingside; break;
          case 'Q': b.castling |= Board::kWhiteQueenside; break;
          case 'k': b.castling |= Board::kBlackKingside; break;
          case 'q': b.castling |= Board::kBlackQueenside; break;
          default: return false;
        }
      }
    }
  }

  // 4) En-passant square.
  if (fields.size() >= 4 && fields[3] != "-") {
    auto &ep = fields[3];
    if (ep.size() != 2) return false;
    auto file = ep[0] - 'a';
    auto rank = ep[1] - '1';
    if (file < 0 || file > 7 || rank != 2 && rank != 5) return false;
    b.ep_square = bitboard::square(rank, file);
  }

  // 5) Halfmove clock.
  if (fields.size() >= 5) {
    auto v = std::strtol(fields[4].c_str(), nullptr, 10);
    if (v < 0) return false;
    b.halfmove_clock = v;
  }

  // 6) Fullmove number.
  if (fields.size() >= 6) {
    auto v = std::strtol(fields[5].c_str(), nullptr, 10);
    if (v < 1) return false;
    b.fullmove = v;
  }

  *this = b;
  return true;
}

std::string Board::to_fen() const {
  auto s = std::string{};
  for (auto rank = 7; rank >= 0; --rank) {
    auto run = 0;
    for (auto file = 0; file < 8; ++file) {
      auto sq = bitboard::square(rank, file);
      Piece p = piece_at(sq);
      if (p == kNumPieces) {
        ++run;
        continue;
      }
      if (run) {
        s += static_cast<char>('0' + run);
        run = 0;
      }
      auto white = color_of_piece(p) == Color::White;
      auto base = piece_char(type_of_piece(p));
      s += white ? base : static_cast<char>(to_lower(base));
    }
    if (run) s += static_cast<char>('0' + run);
    if (rank) s += '/';
  }
  s += " ";
  s += (side_to_move == Color::White) ? "w" : "b";
  s += " ";
  {
    std::string c;
    if (castling & Board::kWhiteKingside) c += 'K';
    if (castling & Board::kWhiteQueenside) c += 'Q';
    if (castling & Board::kBlackKingside) c += 'k';
    if (castling & Board::kBlackQueenside) c += 'q';
    s += c.empty() ? "-" : c;
  }
  s += " ";
  if (ep_square >= 0) {
    s += 'a' + bitboard::file_of(ep_square);
    s += '1' + bitboard::rank_of(ep_square);
  } else {
    s += '-';
  }
  s += " " + std::to_string(halfmove_clock) + " " + std::to_string(fullmove);
  return s;
}

void Board::make_move(const Move& m) {
  auto me = side_to_move;
  auto opp = opponent(me);
  auto mover = piece_at(m.from);
  auto mover_type = type_of_piece(mover);
  auto is_capture = bitboard::test_bit(color_mask(opp), m.to) || m.has_flag(Move::kEnPassant);

  // Remove the moving piece from its origin.
  masks[mover] &= ~bitboard::set_bit(m.from);

  // Remove any opponent piece captured on the destination square. (En-passant
  // captures a pawn on the *side* of the destination, which is handled below.)
  if (!m.has_flag(Move::kEnPassant)) {
    auto to_bit = bitboard::set_bit(m.to);
    auto captured = color_mask(opp) & to_bit;
    if (captured) {
      auto cap = piece_at(m.to);
      masks[cap] &= ~to_bit;
    }
  }

  // Place it on the destination (as a promoted piece if applicable).
  {
    auto dest_piece = (m.promotion != PieceType::None)
                                 ? piece_index(me, m.promotion)
                                 : mover;
    masks[dest_piece] |= bitboard::set_bit(m.to);
  }

  // En-passant removes the captured pawn next to the destination.
  if (m.has_flag(Move::kEnPassant)) {
    auto captured_sq = (me == Color::White) ? m.to - 8 : m.to + 8;
    auto cap = (me == Color::White) ? kBlackPawn : kWhitePawn;
    masks[cap] &= ~bitboard::set_bit(captured_sq);
  }

  // Castling also moves the rook.
  if (m.has_flag(Move::kCastling)) {
    if (me == Color::White) {
      if (m.to == 6) {  // e1-g1 kingside
        masks[kWhiteRook] &= ~bitboard::set_bit(kH1);
        masks[kWhiteRook] |= bitboard::set_bit(kF1);
      } else if (m.to == 2) {  // e1-c1 queenside
        masks[kWhiteRook] &= ~bitboard::set_bit(kA1);
        masks[kWhiteRook] |= bitboard::set_bit(kD1);
      }
    } else {
      if (m.to == 62) {  // e8-g8 kingside
        masks[kBlackRook] &= ~bitboard::set_bit(kH8);
        masks[kBlackRook] |= bitboard::set_bit(kF8);
      } else if (m.to == 58) {  // e8-c8 queenside
        masks[kBlackRook] &= ~bitboard::set_bit(kA8);
        masks[kBlackRook] |= bitboard::set_bit(kD8);
      }
    }
  }

  // Update castling rights: king moved, rook moved, or rook captured.
  if (mover_type == PieceType::King) {
    if (me == Color::White)
      castling &= ~(Board::kWhiteKingside | Board::kWhiteQueenside);
    else
      castling &= ~(Board::kBlackKingside | Board::kBlackQueenside);
  }
  castling &= ~corner_rights(m.from);
  castling &= ~corner_rights(m.to);

  // En-passant target only exists immediately after a double push.
  ep_square = (mover_type == PieceType::Pawn && std::abs(m.to - m.from) == 16)
                  ? ((m.to + m.from) / 2)
                  : -1;

  // Clocks.
  halfmove_clock = (mover_type == PieceType::Pawn || is_capture) ? 0 : halfmove_clock + 1;
  if (side_to_move == Color::Black) ++fullmove;
  side_to_move = opp;
}

}  // namespace chess
