#pragma once

#include <cstdint>

#include <bit>

namespace chess {

using U64 = std::uint64_t;

namespace bitboard {

constexpr U64 kFull = ~0ULL;

// Squares on the a-file and h-file (wrap-around destinations).
constexpr U64 kOffAFile = 0x0101'0101'0101'0101ULL;
constexpr U64 kOffHFile = 0x8080'8080'8080'8080ULL;

// Bit 0 is a1, bit 63 is h8.

inline int popcount(U64 x) { return std::popcount(x); }
inline int ctz(U64 x) { return std::countr_zero(x); }  // Undefined if x == 0.
inline U64 lsb(U64 x) { return x & (0ULL - x); }  // Undefined if x == 0.

inline U64 set_bit(int sq) { return U64{1} << sq; }
inline bool test_bit(U64 bb, int sq) { return (bb & set_bit(sq)) != 0; }

constexpr int rank_of(int sq) { return sq >> 3; }
constexpr int file_of(int sq) { return sq & 7; }
constexpr int square(int rank, int file) { return (rank << 3) | file; }

inline U64 rank_mask(int rank) { return 0xFFULL << (rank * 8); }
inline U64 file_mask(int file) { return 0x0101'0101'0101'0101ULL << file; }

// Shifts every bit of `bb` by (dr, df) and masks off squares that wrapped
// around the board edge. `dr` and `df` are in {-1, 0, +1}, not both zero.
inline U64 shift_dir(U64 bb, int dr, int df) {
  const int delta = dr * 8 + df;
  U64 r = (delta > 0) ? (bb << delta) : (bb >> -delta);
  if (df == 1) r &= ~kOffAFile;   // h-file pieces wrap to a-file
  if (df == -1) r &= ~kOffHFile;  // a-file pieces wrap to h-file
  return r;
}

}  // namespace bitboard

}  // namespace chess
