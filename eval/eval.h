#pragma once

#include "movegen/board.h"

namespace chess {

class Evaluator {
 public:
  virtual ~Evaluator() = default;
  virtual int evaluate(const Board& b) const = 0;
};

class MaterialEvaluator final : public Evaluator {
 public:
  int evaluate(const Board& b) const override;
};

}  // namespace chess
