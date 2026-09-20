#pragma once

#include "search.h"

namespace chess {

template <typename Evaluator>
class IterativeDeepener {
 public:
  explicit IterativeDeepener(Evaluator eval) : search_(std::move(eval)) {}

  SearchResult search(Board& b, int maxDepth) {
    SearchResult best{0, Move{}, 0};
    for (int d = 1; d <= maxDepth; ++d) {
      auto res = search_.search(b, d);
      best = res;
    }
    return best;
  }

  SearchResult search_depth(Board& b, int depth) {
    return search_.search(b, depth);
  }

 private:
  Search<Evaluator> search_;
};

}  // namespace chess
