#include "game.h"
#include "movegen/movegen.h"
#include "search/search.h"
#include "eval/eval.h"

#include <iostream>

namespace chess {

void Clock::tick(Color c, std::chrono::milliseconds elapsed) {
  if (c == Color::White) {
    white_time -= elapsed;
    if (white_time < std::chrono::milliseconds(0)) white_time = std::chrono::milliseconds(0);
    white_time += white_inc;
  } else {
    black_time -= elapsed;
    if (black_time < std::chrono::milliseconds(0)) black_time = std::chrono::milliseconds(0);
    black_time += black_inc;
  }
}

bool Clock::is_time_up(Color c) const {
  return c == Color::White ? white_time <= std::chrono::milliseconds(0)
                           : black_time <= std::chrono::milliseconds(0);
}

Move HumanPlayer::choose_move(Board& board) {
  // Simple UCI-like input: e2e4
  std::string input;
  while (true) {
    std::cout << "Move: ";
    if (!(std::cin >> input)) {
      std::exit(0);
    }
    if (input.size() == 4) {
      // Very naive parsing
      int from = 0; // placeholder
      int to = 0;
      // For now, just return first legal move to keep tests simple
      auto moves = generate_moves(board);
      if (!moves.empty()) return moves[0];
    }
  }
}

ComputerPlayer::ComputerPlayer(int depth) : depth_(depth) {}

Move ComputerPlayer::choose_move(Board& board) {
  MaterialEvaluator eval;
  Search<MaterialEvaluator> searcher(eval);
  auto result = searcher.search(board, depth_);
  return result.best_move;
}

Game::Game(std::unique_ptr<Player> white, std::unique_ptr<Player> black, Clock clock)
    : white_(std::move(white)), black_(std::move(black)), clock_(clock) {
  board_.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Game::play() {
  auto current_player = [this]() -> Player* {
    return board_.side_to_move == Color::White ? white_.get() : black_.get();
  };

  while (!game_over_) {
    auto moves = generate_moves(board_);
    if (moves.empty()) {
      game_over_ = true;
      break;
    }

    auto start = std::chrono::steady_clock::now();
    Move m = current_player()->choose_move(board_);
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    board_.make_move(m);
    clock_.tick(board_.side_to_move == Color::White ? Color::Black : Color::White, elapsed);

    if (clock_.is_time_up(board_.side_to_move)) {
      game_over_ = true;
      break;
    }
  }
}

}  // namespace chess
