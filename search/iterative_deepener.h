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
    Board tmp = b;
    for (int d = 1; d <= maxDepth; ++d) {
      auto res = search_.search(tmp, d);
      best = res;
      best.pv.insert(best.pv.end(), res.pv.begin(), res.pv.end());
      // Actually we want cumulative PV
      // Build PV incrementally
      if (res.best_move.from != -1) {
        best.pv.push_back(res.best_move);
        tmp.make_move(res.best_move);
      }
      if (cb) cb(d, res.score, res.nodes, res.best_move);
    }
    return best;
  }

  SearchResult search_depth(Board& b, int depth, std::function<void(int,int,int,Move)> cb = nullptr) {
    auto res = search_.search(b, depth);
    if (cb) cb(depth, res.score, res.nodes, res.best_move);
    return res;
  }

 private:
  Search<Evaluator> search_;
};

}  // namespace chess
