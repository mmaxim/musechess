#include "game/game.h"
#include "movegen/board.h"
#include "movegen/movegen.h"
#include "search/search.h"
#include "eval/eval.h"

#include <iostream>
#include <string>
#include <memory>

using namespace chess;

void print_board(const Board& b) {
  for (int r = 7; r >= 0; --r) {
    std::cout << r + 1 << " ";
    for (int f = 0; f < 8; ++f) {
      int sq = r * 8 + f;
      auto p = b.piece_at(sq);
      char c = '.';
      if (p != static_cast<Piece>(kNumPieces)) {
        c = (color_of_piece(p) == Color::White ? 'w' : 'b');
      }
      std::cout << c << ' ';
    }
    std::cout << '\n';
  }
  std::cout << "  a b c d e f g h\n";
}

int sq_from_str(const std::string& s) {
  if (s.size() < 2) return -1;
  int file = s[0] - 'a';
  int rank = s[1] - '1';
  if (file <0 || file>7 || rank<0 || rank>7) return -1;
  return rank*8 + file;
}

int main() {
  std::cout << "Chess CLI - Human vs Computer\n";
  std::cout << "White is human, Black is computer\n\n";

  Board board;
  board.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  MaterialEvaluator eval;
  Search<MaterialEvaluator> searcher(eval);

  while (true) {
    print_board(board);
    std::cout << (board.side_to_move == Color::White ? "White" : "Black") << " to move\n";
    if (board.side_to_move == Color::White) {
      std::cout << "Enter move e2e4: ";
      std::string in;
      if (!(std::cin >> in)) break;
      if (in == "quit") break;
      if (in.size() < 4) { std::cout << "Invalid\n"; continue; }
      int from = sq_from_str(in.substr(0,2));
      int to   = sq_from_str(in.substr(2,2));
      if (from <0 || to <0) { std::cout << "Invalid squares\n"; continue; }
      MoveList moves = generate_moves(board);
      bool ok = false;
      for (int i=0;i<moves.size();++i) if (moves[i].from==from && moves[i].to==to) ok=true;
      if (!ok) { std::cout << "Illegal move\n"; continue; }
      Move m; m.from=from; m.to=to;
      board.make_move(m);
      std::cout << "You played " << m.to_string() << "\n";
    } else {
      std::cout << "Engine thinking...\n";
      auto res = searcher.search(board, 3);
      std::cout << "Engine plays " << res.best_move.to_string()
                << " score " << res.score
                << " nodes " << res.nodes << "\n";
      board.make_move(res.best_move);
    }
  }
  return 0;
}
