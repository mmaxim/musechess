#pragma once

#include "movegen/board.h"
#include "movegen/move.h"
#include <chrono>
#include <memory>

namespace chess {

class Player {
 public:
  virtual ~Player() = default;
  virtual Move choose_move(Board& board) = 0;
};

class HumanPlayer final : public Player {
 public:
  Move choose_move(Board& board) override;
};

class ComputerPlayer final : public Player {
 public:
  explicit ComputerPlayer(int depth = 3);
  Move choose_move(Board& board) override;

 private:
  int depth_ = 3;
};

struct Clock {
  std::chrono::milliseconds white_time{std::chrono::minutes(5)};
  std::chrono::milliseconds black_time{std::chrono::minutes(5)};
  std::chrono::milliseconds white_inc{0};
  std::chrono::milliseconds black_inc{0};

  void set_time(Color c, std::chrono::milliseconds t) {
    if (c == Color::White) white_time = t; else black_time = t;
  }
  void set_inc(Color c, std::chrono::milliseconds inc) {
    if (c == Color::White) white_inc = inc; else black_inc = inc;
  }
  void tick(Color c, std::chrono::milliseconds elapsed);
  bool is_time_up(Color c) const;
};

class Game {
 public:
  Game(std::unique_ptr<Player> white, std::unique_ptr<Player> black, Clock clock = {});
  void play(); // simple loop, returns when game over

  Board& board() { return board_; }
  const Board& board() const { return board_; }

 private:
  Board board_;
  std::unique_ptr<Player> white_;
  std::unique_ptr<Player> black_;
  Clock clock_;
  bool game_over_ = false;
};

}  // namespace chess
