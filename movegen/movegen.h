#pragma once

#include "board.h"
#include "move.h"

namespace chess {

// All squares attacked by any piece of color `c`.
U64 attacked_by(const Board& b, Color c);

// Whether the side to move is in check.
bool in_check(const Board& b);

// Pseudo-legal moves: everything that follows the piece movement rules,
// including moves that leave own king en prise.
MoveList generate_pseudo_legal(const Board& b);

// Legal moves: pseudo-legal moves with king-safety filtered out.
MoveList generate_moves(const Board& b);

}  // namespace chess
