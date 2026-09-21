#pragma once

#include "search.h"

namespace chess {

template <typename Evaluator>
class IterativeDeepener {
 public:
  explicit IterativeDeepener(Evaluator eval) : search_(std::move(eval)) {}

  void set_stop_flag(std::atomic<bool>* flag) { search_.set_stop_flag(flag); }

  SearchResult search(Board& b, int maxDepth, std::function<void(const SearchResult&)> cb = nullptr) {
    SearchResult best{0, Move{}, 0};
    for (int d = 1; d <= maxDepth; ++d) {
      auto res = search_.search(b, d, cb);
      best = res;
    }
    return best;
  }

  SearchResult search_depth(Board& b, int depth, std::function<void(const SearchResult&)> cb = nullptr) {
    return search_.search(b, depth, cb);
  }

 private:
  Search<Evaluator> search_;
};

}  // namespace chess
