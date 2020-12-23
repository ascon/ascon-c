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

#define ASCON_128_IV WORD_T(0x8021000008220000)
#define ASCON_128A_IV WORD_T(0x8822000000200000)
#define ASCON_80PQ_IV WORD_T(0xc021000008220000)
#define ASCON_HASH_IV WORD_T(0x0020000008020010)
#define ASCON_XOF_IV WORD_T(0x0020000008020000)

#define ASCON_HASH_IV0 WORD_T(0xf9afb5c6a540dbc7)
#define ASCON_HASH_IV1 WORD_T(0xbd2493011445a340)
#define ASCON_HASH_IV2 WORD_T(0xcb9ba8b5604d4fc8)
#define ASCON_HASH_IV3 WORD_T(0x12a4eede94514c98)
#define ASCON_HASH_IV4 WORD_T(0x4bca84c06339f398)

#define ASCON_XOF_IV0 WORD_T(0xc75782817e351ae6)
#define ASCON_XOF_IV1 WORD_T(0x70045f441d238220)
#define ASCON_XOF_IV2 WORD_T(0x5dd5ab52a13e3f04)
#define ASCON_XOF_IV3 WORD_T(0x3e378142c30c1db2)
#define ASCON_XOF_IV4 WORD_T(0x3735189db624d656)

#define START(n) (12 - n)
#define RC(e, o) WORD_T((uint64_t)o << 32 | e)

forceinline void P12ROUNDS(state_t* s) {
  ROUND(s, RC(0xc, 0xc));
  ROUND(s, RC(0x9, 0xc));
  ROUND(s, RC(0xc, 0x9));
  ROUND(s, RC(0x9, 0x9));
  ROUND(s, RC(0x6, 0xc));
  ROUND(s, RC(0x3, 0xc));
  ROUND(s, RC(0x6, 0x9));
  ROUND(s, RC(0x3, 0x9));
  ROUND(s, RC(0xc, 0x6));
  ROUND(s, RC(0x9, 0x6));
  ROUND(s, RC(0xc, 0x3));
  ROUND(s, RC(0x9, 0x3));
}

forceinline void P8ROUNDS(state_t* s) {
  ROUND(s, RC(0x6, 0xc));
  ROUND(s, RC(0x3, 0xc));
  ROUND(s, RC(0x6, 0x9));
  ROUND(s, RC(0x3, 0x9));
  ROUND(s, RC(0xc, 0x6));
  ROUND(s, RC(0x9, 0x6));
  ROUND(s, RC(0xc, 0x3));
  ROUND(s, RC(0x9, 0x3));
}

forceinline void P6ROUNDS(state_t* s) {
  ROUND(s, RC(0x6, 0x9));
  ROUND(s, RC(0x3, 0x9));
  ROUND(s, RC(0xc, 0x6));
  ROUND(s, RC(0x9, 0x6));
  ROUND(s, RC(0xc, 0x3));
  ROUND(s, RC(0x9, 0x3));
}

extern const uint8_t constants[][2];

forceinline void PROUNDS(state_t* s, int nr) {
  for (int i = START(nr); i < 12; i++)
    ROUND(s, RC(constants[i][0], constants[i][1]));
}

#if ASCON_INLINE_PERM && ASCON_UNROLL_LOOPS

forceinline void P(state_t* s, int nr) {
  if (nr == 12) P12ROUNDS(s);
  if (nr == 8) P8ROUNDS(s);
  if (nr == 6) P6ROUNDS(s);
}

#elif !ASCON_INLINE_PERM && ASCON_UNROLL_LOOPS

void P12(state_t* s);
void P8(state_t* s);
void P6(state_t* s);

forceinline void P(state_t* s, int nr) {
  if (nr == 12) P12(s);
  if (nr == 8) P8(s);
  if (nr == 6) P6(s);
}

#elif ASCON_INLINE_PERM && !ASCON_UNROLL_LOOPS

forceinline void P(state_t* s, int nr) { PROUNDS(s, nr); }

#else /* !ASCON_INLINE_PERM && !ASCON_UNROLL_LOOPS */

void P(state_t* s, int nr);

#endif

#endif /* PERMUTATIONS_H_ */
