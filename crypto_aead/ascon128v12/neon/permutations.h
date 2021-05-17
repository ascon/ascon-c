#ifndef PERMUTATIONS_H_
#define PERMUTATIONS_H_

#include <stdint.h>

#include "api.h"
#include "ascon.h"
#include "config.h"
#include "printstate.h"
#include "round.h"

#define ASCON_128_KEYBYTES 16
#define ASCON_128A_KEYBYTES 16
#define ASCON_80PQ_KEYBYTES 20

#define ASCON_128_RATE 8
#define ASCON_128A_RATE 16

#define ASCON_128_PA_ROUNDS 12
#define ASCON_128_PB_ROUNDS 6
#define ASCON_128A_PB_ROUNDS 8

#define ASCON_HASH_BYTES 32

#define ASCON_128_IV WORD_T(0x80400c0600000000ull)
#define ASCON_128A_IV WORD_T(0x80800c0800000000ull)
#define ASCON_80PQ_IV WORD_T(0xa0400c0600000000ull)
#define ASCON_HASH_IV WORD_T(0x00400c0000000100ull)
#define ASCON_XOF_IV WORD_T(0x00400c0000000000ull)

#define ASCON_HASH_IV0 WORD_T(0xee9398aadb67f03dull)
#define ASCON_HASH_IV1 WORD_T(0x8bb21831c60f1002ull)
#define ASCON_HASH_IV2 WORD_T(0xb48a92db98d5da62ull)
#define ASCON_HASH_IV3 WORD_T(0x43189921b8f8e3e8ull)
#define ASCON_HASH_IV4 WORD_T(0x348fa5c9d525e140ull)

#define ASCON_XOF_IV0 WORD_T(0xb57e273b814cd416ull)
#define ASCON_XOF_IV1 WORD_T(0x2b51042562ae2420ull)
#define ASCON_XOF_IV2 WORD_T(0x66a3a7768ddf2218ull)
#define ASCON_XOF_IV3 WORD_T(0x5aad0a7a8153650cull)
#define ASCON_XOF_IV4 WORD_T(0x4f3e0e32539493b6ull)

#define START(n) ((3 + (n)) << 4 | (12 - (n)))
#define RC(c) WORD_T(c)

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

forceinline void P12(state_t* s) {
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

forceinline void P8(state_t* s) {
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

forceinline void P6(state_t* s) {
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

forceinline void P(state_t* s, int nr) {
  if (nr == 12) P12(s);
  if (nr == 8) P8(s);
  if (nr == 6) P6(s);
}

#endif /* PERMUTATIONS_H_ */
