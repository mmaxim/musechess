#include "eval.h"

namespace chess {

namespace {
constexpr int kMaterial[12] = {
    100, 320, 330, 500, 900, 20000,
    100, 320, 330, 500, 900, 20000
};
}

int MaterialEvaluator::evaluate(const Board& b) const {
  auto score = 0;
  for (auto i = 0; i < kNumPieces; ++i) {
    auto mask = b.masks[i];
    while (mask) {
      mask &= mask - 1;
      auto val = kMaterial[i];
      if (color_of_piece(static_cast<Piece>(i)) == Color::White) {
        score += val;
      } else {
        score -= val;
      }
    }
  }
  return b.side_to_move == Color::White ? score : -score;
}

}  // namespace chess
