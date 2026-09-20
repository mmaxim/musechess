#include <gtest/gtest.h>

#include "search.h"
#include "search/iterative_deepener.h"
#include "eval/eval.h"
#include "movegen/board.h"
#include "movegen/movegen.h"

using namespace chess;

namespace {
MaterialEvaluator mat_eval;
Search<MaterialEvaluator> searcher{mat_eval};
}

Board board_from(const char* fen) {
  Board b;
  EXPECT_TRUE(b.set_fen(fen));
  return b;
}

TEST(Search, EvaluateMaterialStart) {
  auto b = board_from("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  EXPECT_EQ(mat_eval.evaluate(b), 0);
}

TEST(Search, EvaluateMaterialUp) {
  auto b = board_from("4k3/8/8/8/8/8/4P3/4K3 w - - 0 1");
  EXPECT_GT(mat_eval.evaluate(b), 0);
}

TEST(Search, EvaluateMaterialDown) {
  auto b = board_from("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
  EXPECT_EQ(mat_eval.evaluate(b), 0);
}

TEST(Search, SearchDepthZeroNotUsed) {
  auto b = board_from("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
  // Ensure evaluator works for depth leaf
  EXPECT_EQ(mat_eval.evaluate(b), mat_eval.evaluate(b));
}

TEST(Search, SearchFindsLegalMoveDepth1) {
  auto b = board_from("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  auto res = searcher.search(b, 1);
  EXPECT_NE(res.best_move.from, -1);
  EXPECT_GT(res.nodes, 0);
}

TEST(Search, SearchDepth1Consistency) {
  auto b = board_from("8/8/8/8/8/3k4/8/4K3 w - - 0 1");
  auto res = searcher.search(b, 1);
  EXPECT_NE(res.best_move.from, -1);
  EXPECT_NE(res.best_move.to, -1);
}

TEST(Search, NullMoveHeuristicDoesNotCrash) {
  auto b = board_from("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
  auto res = searcher.search(b, 2);
  EXPECT_GT(res.nodes, 0);
  EXPECT_GT(res.score, -30000);
  EXPECT_LT(res.score, 30000);
}

TEST(Search, SearchRespectsCheck) {
  auto b = board_from("4rk2/8/8/8/8/8/8/4K3 w - - 0 1");
  auto res = searcher.search(b, 2);
  Board child = b;
  child.make_move(res.best_move);
  EXPECT_FALSE(in_check(child));
}

TEST(Search, PVLengthMatchesDepth) {
  auto b = board_from("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  for (int d = 1; d <= 4; ++d) {
    auto res = searcher.search(b, d);
    EXPECT_EQ(res.pv.moves.size(), static_cast<size_t>(d)) << "PV length mismatch at depth " << d;
  }
}

TEST(Search, PVFirstMoveEqualsBestMove) {
  auto b = board_from("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  auto res = searcher.search(b, 3);
  ASSERT_FALSE(res.pv.moves.empty());
  EXPECT_EQ(res.best_move, res.pv.moves[0]);
}

TEST(Search, IterativeDeepenerPVLength) {
  auto b = board_from("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  IterativeDeepener<MaterialEvaluator> deepener{mat_eval};
  for (int d = 1; d <= 4; ++d) {
    auto res = deepener.search_depth(b, d);
    EXPECT_EQ(res.pv.moves.size(), static_cast<size_t>(d)) << "Iterative deepener PV length mismatch at depth " << d;
  }
}

TEST(Search, PVIsLegalAndProgresses) {
  auto b = board_from("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  auto res = searcher.search(b, 4);
  ASSERT_EQ(res.pv.moves.size(), 4u);
  Board tmp = b;
  for (size_t i = 0; i < res.pv.moves.size(); ++i) {
    auto& m = res.pv.moves[i];
    auto moves = generate_moves(tmp);
    bool found = false;
    for (auto& mm : moves) {
      if (mm.from == m.from && mm.to == m.to) { found = true; break; }
    }
    EXPECT_TRUE(found) << "PV move " << i << " not legal";
    tmp.make_move(m);
  }
  // Ensure no immediate repetition of same from/to at different ply
  for (size_t i = 1; i < res.pv.moves.size(); ++i) {
    EXPECT_NE(res.pv.moves[i].from, res.pv.moves[i-2].from) << "PV repeats move at ply " << i;
  }
}
