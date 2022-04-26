#ifndef PERMUTATIONS_H_
#define PERMUTATIONS_H_

#include <stdint.h>

#include "api.h"
#include "ascon.h"
#include "config.h"
#include "constants.h"
#include "printstate.h"
#include "round.h"

const uint64_t C[12] = {
    0xffffffffffffff0full, 0xffffffffffffff1eull, 0xffffffffffffff2dull,
    0xffffffffffffff3cull, 0xffffffffffffff4bull, 0xffffffffffffff5aull,
    0xffffffffffffff69ull, 0xffffffffffffff78ull, 0xffffffffffffff87ull,
    0xffffffffffffff96ull, 0xffffffffffffffa5ull, 0xffffffffffffffb4ull,
};

#define P12ROUNDS(s) \
  ROUND(0)           \
  ROUND(8)           \
  ROUND(16)          \
  ROUND(24)          \
  ROUND(32)          \
  ROUND(40)          \
  ROUND(48)          \
  ROUND(56)          \
  ROUND(64)          \
  ROUND(72)          \
  ROUND(80)          \
  ROUND(88)

#define P8ROUNDS(s) \
  ROUND(32)         \
  ROUND(40)         \
  ROUND(48)         \
  ROUND(56)         \
  ROUND(64)         \
  ROUND(72)         \
  ROUND(80)         \
  ROUND(88)

#define P6ROUNDS(s) \
  ROUND(48)         \
  ROUND(56)         \
  ROUND(64)         \
  ROUND(72)         \
  ROUND(80)         \
  ROUND(88)

forceinline void P12(ascon_state_t* s) {
  __asm__ __volatile__ ( \
      ".arm \n\t" \
      ".fpu neon \n\t" \
      "vldm %[s], {d0-d4} \n\t" \
      "vmvn d2, d2 \n\t" \
      P12ROUNDS(s) \
      "vmvn d2, d2 \n\t" \
      "vstm %[s], {d0-d4} \n\t" \
      :: [s] "r" (s), [C] "r" (C) \
      : "d0", "d1", "d2", "d3", "d4", \
        "d10", "d11", "d12", "d13", "d14", \
        "d20", "d21", "d22", "d23", "d24", \
        "d31", "memory");
}

forceinline void P8(ascon_state_t* s) {
  __asm__ __volatile__ ( \
      ".arm \n\t" \
      ".fpu neon \n\t" \
      "vldm %[s], {d0-d4} \n\t" \
      "vmvn d2, d2 \n\t" \
      P8ROUNDS(s) \
      "vmvn d2, d2 \n\t" \
      "vstm %[s], {d0-d4} \n\t" \
      :: [s] "r" (s), [C] "r" (C) \
      : "d0", "d1", "d2", "d3", "d4", \
        "d10", "d11", "d12", "d13", "d14", \
        "d20", "d21", "d22", "d23", "d24", \
        "d31", "memory");
}

forceinline void P6(ascon_state_t* s) {
  __asm__ __volatile__ ( \
      ".arm \n\t" \
      ".fpu neon \n\t" \
      "vldm %[s], {d0-d4} \n\t" \
      "vmvn d2, d2 \n\t" \
      P6ROUNDS(s) \
      "vmvn d2, d2 \n\t" \
      "vstm %[s], {d0-d4} \n\t" \
      :: [s] "r" (s), [C] "r" (C) \
      : "d0", "d1", "d2", "d3", "d4", \
        "d10", "d11", "d12", "d13", "d14", \
        "d20", "d21", "d22", "d23", "d24", \
        "d31", "memory");
}

forceinline void P(ascon_state_t* s, int nr) {
  if (nr == 12) P12(s);
  if (nr == 8) P8(s);
  if (nr == 6) P6(s);
}

#endif /* PERMUTATIONS_H_ */
