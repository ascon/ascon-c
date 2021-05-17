#ifndef PERMUTATIONS_H_
#define PERMUTATIONS_H_

#include <stdint.h>

#include "ascon.h"
#include "printstate.h"
#include "round.h"

#define ASCON_128_KEYBYTES 16
#define ASCON_128A_KEYBYTES 16
#define ASCON_80PQ_KEYBYTES 20

#define ASCON_128_RATE 8
#define ASCON_128A_RATE 16
#define ASCON_HASH_RATE 8

#define ASCON_128_PA_ROUNDS 12
#define ASCON_128_PB_ROUNDS 6

#define ASCON_128A_PA_ROUNDS 12
#define ASCON_128A_PB_ROUNDS 8

#define ASCON_HASH_PA_ROUNDS 12
#define ASCON_HASH_PB_ROUNDS 12

#define ASCON_HASHA_PA_ROUNDS 12
#define ASCON_HASHA_PB_ROUNDS 8

#define ASCON_HASH_BYTES 32

#define ASCON_128_IV                            \
  (((uint64_t)(ASCON_128_KEYBYTES * 8) << 56) | \
   ((uint64_t)(ASCON_128_RATE * 8) << 48) |     \
   ((uint64_t)(ASCON_128_PA_ROUNDS) << 40) |    \
   ((uint64_t)(ASCON_128_PB_ROUNDS) << 32))

#define ASCON_128A_IV                            \
  (((uint64_t)(ASCON_128A_KEYBYTES * 8) << 56) | \
   ((uint64_t)(ASCON_128A_RATE * 8) << 48) |     \
   ((uint64_t)(ASCON_128A_PA_ROUNDS) << 40) |    \
   ((uint64_t)(ASCON_128A_PB_ROUNDS) << 32))

#define ASCON_80PQ_IV                            \
  (((uint64_t)(ASCON_80PQ_KEYBYTES * 8) << 56) | \
   ((uint64_t)(ASCON_128_RATE * 8) << 48) |      \
   ((uint64_t)(ASCON_128_PA_ROUNDS) << 40) |     \
   ((uint64_t)(ASCON_128_PB_ROUNDS) << 32))

#define ASCON_HASH_IV                                                \
  (((uint64_t)(ASCON_HASH_RATE * 8) << 48) |                         \
   ((uint64_t)(ASCON_HASH_PA_ROUNDS) << 40) |                        \
   ((uint64_t)(ASCON_HASH_PA_ROUNDS - ASCON_HASH_PB_ROUNDS) << 32) | \
   ((uint64_t)(ASCON_HASH_BYTES * 8) << 0))

#define ASCON_HASHA_IV                                                 \
  (((uint64_t)(ASCON_HASH_RATE * 8) << 48) |                           \
   ((uint64_t)(ASCON_HASHA_PA_ROUNDS) << 40) |                         \
   ((uint64_t)(ASCON_HASHA_PA_ROUNDS - ASCON_HASHA_PB_ROUNDS) << 32) | \
   ((uint64_t)(ASCON_HASH_BYTES * 8) << 0))

#define ASCON_XOF_IV                          \
  (((uint64_t)(ASCON_HASH_RATE * 8) << 48) |  \
   ((uint64_t)(ASCON_HASH_PA_ROUNDS) << 40) | \
   ((uint64_t)(ASCON_HASH_PA_ROUNDS - ASCON_HASH_PB_ROUNDS) << 32))

#define ASCON_XOFA_IV                          \
  (((uint64_t)(ASCON_HASH_RATE * 8) << 48) |   \
   ((uint64_t)(ASCON_HASHA_PA_ROUNDS) << 40) | \
   ((uint64_t)(ASCON_HASHA_PA_ROUNDS - ASCON_HASHA_PB_ROUNDS) << 32))

static inline void P12(state_t* s) {
  printstate(" permutation input", s);
  ROUND(s, 0xf0);
  ROUND(s, 0xe1);
  ROUND(s, 0xd2);
  ROUND(s, 0xc3);
  ROUND(s, 0xb4);
  ROUND(s, 0xa5);
  ROUND(s, 0x96);
  ROUND(s, 0x87);
  ROUND(s, 0x78);
  ROUND(s, 0x69);
  ROUND(s, 0x5a);
  ROUND(s, 0x4b);
}

static inline void P8(state_t* s) {
  printstate(" permutation input", s);
  ROUND(s, 0xb4);
  ROUND(s, 0xa5);
  ROUND(s, 0x96);
  ROUND(s, 0x87);
  ROUND(s, 0x78);
  ROUND(s, 0x69);
  ROUND(s, 0x5a);
  ROUND(s, 0x4b);
}

static inline void P6(state_t* s) {
  printstate(" permutation input", s);
  ROUND(s, 0x96);
  ROUND(s, 0x87);
  ROUND(s, 0x78);
  ROUND(s, 0x69);
  ROUND(s, 0x5a);
  ROUND(s, 0x4b);
}

#endif /* PERMUTATIONS_H_ */
