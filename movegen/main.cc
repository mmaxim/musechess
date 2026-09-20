// Demo: load a position (FEN or "start"), print the board, list all legal moves.
//
//   chess-demo                      # start position
//   chess-demo start
//   chess-demo "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
//   chess-demo <file>               # first non-comment line of file

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>

#include "board.h"
#include "movegen.h"

namespace {

std::string print_board(const chess::Board& b) {
  std::string s;
  for (int rank = 7; rank >= 0; --rank) {
    s += std::to_string(rank + 1) + " |";
    for (int file = 0; file < 8; ++file) {
      const int sq = rank * 8 + file;
      const chess::Piece p = b.piece_at(sq);
      char c = '.';
      if (p != chess::kNumPieces) {
        c = chess::piece_char(chess::type_of_piece(p));
        if (chess::color_of_piece(p) == chess::Color::Black) c = std::tolower(c);
      }
      s += " " + std::string(1, c) + " ";
    }
    s += "|\n";
  }
  s += "    a  b  c  d  e  f  g  h\n";
  return s;
}

}  // namespace

int main(int argc, char** argv) {
  using namespace chess;

  std::string fen =
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

  if (argc >= 2) {
    std::string arg = argv[1];
    if (arg != "start") {
      // Treat as FEN; if it looks like a path, read its first non-comment line.
      std::ifstream in(arg);
      if (in) {
        std::string line;
        while (std::getline(in, line)) {
          if (!line.empty() && line.front() != ';') {
            arg = line;
            break;
          }
        }
      }
    } else {
      arg.clear();
    }
    if (!arg.empty()) fen = arg;
  }

  Board b;
  if (!b.set_fen(fen)) {
    std::cerr << "error: failed to parse FEN: " << fen << "\n";
    return 1;
  }

  std::cout << print_board(b);
  std::cout << "FEN: " << b.to_fen() << "\n";
  if (in_check(b)) std::cout << "Side to move is in check.\n";

  const MoveList moves = generate_moves(b);
  std::cout << moves.size() << " legal moves:\n";
  for (const Move& m : moves) {
    std::cout << "  " << m.to_string() << "\n";
  }
  return 0;
}
