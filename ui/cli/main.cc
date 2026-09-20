#include "game/game.h"
#include "movegen/board.h"
#include "movegen/movegen.h"

#include <iostream>
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
        // Very simple symbol
        c = 'p';
      }
      std::cout << c << ' ';
    }
    std::cout << '\n';
  }
  std::cout << "  a b c d e f g h\n";
}

int main() {
  std::cout << "Chess CLI - Human vs Computer\n";
  std::cout << "White is human, Black is computer\n\n";

  auto white = std::make_unique<HumanPlayer>();
  auto black = std::make_unique<ComputerPlayer>(3);
  Clock clock;
  clock.white_time = std::chrono::minutes(5);
  clock.black_time = std::chrono::minutes(5);

  Game game(std::move(white), std::move(black), clock);

  print_board(game.board());
  std::cout << "Game started. Enter moves as e2e4. Type 'quit' to exit.\n";

  // Simple loop for demonstration
  while (true) {
    auto moves = generate_moves(game.board());
    if (moves.empty()) {
      std::cout << "Game over.\n";
      break;
    }
    // For now just show possible moves count
    std::cout << "Legal moves: " << moves.size() << "\n";
    break; // avoid infinite loop in demo
  }

  return 0;
}
