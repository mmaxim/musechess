#include <gtest/gtest.h>
#include "uci.h"

using namespace chess::uci;

TEST(UCI, UciNewGameResetsBoard) {
  Engine e;
  e.uci_new_game();
  EXPECT_EQ(e.best_move(), "");
}

TEST(UCI, PositionSetsFEN) {
  Engine e;
  e.position("8/8/8/8/8/8/8/4K3 w - - 0 1", {});
  e.go(1);
  EXPECT_NE(e.best_move(), ""); // just ensure no crash
}

TEST(UCI, GoFindsBestMove) {
  Engine e;
  e.uci_new_game();
  e.go(1);
  auto bm = e.best_move();
  EXPECT_FALSE(bm.empty());
}

TEST(UCI, SetOptionDepth) {
  Engine e;
  e.set_option("Depth", "6");
  e.go(); // should use default depth
  EXPECT_FALSE(e.best_move().empty());
}
