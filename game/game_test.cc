#include <gtest/gtest.h>
#include "game.h"
#include "movegen/board.h"

using namespace chess;

TEST(Game, ClockTick) {
  Clock c;
  c.white_time = std::chrono::seconds(10);
  c.set_inc(Color::White, std::chrono::seconds(1));
  c.tick(Color::White, std::chrono::seconds(3));
  EXPECT_EQ(c.white_time.count(), 8000);
}

TEST(Game, ClockTimeUp) {
  Clock c;
  c.black_time = std::chrono::seconds(0);
  EXPECT_TRUE(c.is_time_up(Color::Black));
  EXPECT_FALSE(c.is_time_up(Color::White));
}

TEST(Game, GameInitializesBoard) {
  auto white = std::make_unique<HumanPlayer>();
  auto black = std::make_unique<HumanPlayer>();
  Game g(std::move(white), std::move(black));
  EXPECT_EQ(g.board().side_to_move, Color::White);
}
