#pragma once

#include "search.h"
#include <functional>

namespace chess {

template <typename Evaluator>
class IterativeDeepener {
 public:
  explicit IterativeDeepener(Evaluator eval) : search_(std::move(eval)) {}

  SearchResult search(Board& b, int maxDepth, std::function<void(int,int,int,Move)> cb = nullptr) {
    SearchResult best{0, Move{}, 0};
    for (int d = 1; d <= maxDepth; ++d) {
      auto res = search_.search(b, d);
      best = res;
      if (cb) cb(d, res.score, res.nodes, res.best_move);
    }
    return best;
  }

 private:
  Search<Evaluator> search_;
};

}  // namespace chess
