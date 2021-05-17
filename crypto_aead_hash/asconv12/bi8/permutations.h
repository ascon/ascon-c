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

#define ASCON_128_IV WORD_T(0x8040000020301000ull)
#define ASCON_128A_IV WORD_T(0xc000000030200000ull)
#define ASCON_80PQ_IV WORD_T(0x8040800020301000ull)
#define ASCON_HASH_IV WORD_T(0x0040000020200002ull)
#define ASCON_XOF_IV WORD_T(0x0040000020200000ull)

#define ASCON_HASH_IV0 WORD_T(0xfa8e976bb985dc4dull)
#define ASCON_HASH_IV1 WORD_T(0xc8085072a40ccd94ull)
#define ASCON_HASH_IV2 WORD_T(0xfe1781be5a847314ull)
#define ASCON_HASH_IV3 WORD_T(0x2f871f6c6d0082b2ull)
#define ASCON_HASH_IV4 WORD_T(0x7a1ba68850ec407eull)

#define ASCON_HASHA_IV0 WORD_T(0x194c0f180a5d41e4ull)
#define ASCON_HASHA_IV1 WORD_T(0x7faa87825647f3a7ull)
#define ASCON_HASHA_IV2 WORD_T(0x606dbe06db8da430ull)
#define ASCON_HASHA_IV3 WORD_T(0xe0dd6bcf19fbce3bull)
#define ASCON_HASHA_IV4 WORD_T(0x9720dc4446473d8bull)

#define ASCON_XOF_IV0 WORD_T(0x8a46f0d354e771b8ull)
#define ASCON_XOF_IV1 WORD_T(0x04489f4084368cd0ull)
#define ASCON_XOF_IV2 WORD_T(0x6c94f2150dbcf66cull)
#define ASCON_XOF_IV3 WORD_T(0x48965294f143b44eull)
#define ASCON_XOF_IV4 WORD_T(0x0788515fe0e5fb8aull)

#define ASCON_XOFA_IV0 WORD_T(0x4ab43d4f16a80d2cull)
#define ASCON_XOFA_IV1 WORD_T(0xd0ae310bf0f619ceull)
#define ASCON_XOFA_IV2 WORD_T(0xc08cf3c801d89cf3ull)
#define ASCON_XOFA_IV3 WORD_T(0x3859d2094dac0b35ull)
#define ASCON_XOFA_IV4 WORD_T(0xd274992be52b5357ull)

#define START(n) (12 - n)
#define RC(c) WORD_T(c)

forceinline void P12ROUNDS(state_t* s) {
  ROUND(s, RC(0x0101010100000000ull));
  ROUND(s, RC(0x0101010000000001ull));
  ROUND(s, RC(0x0101000100000100ull));
  ROUND(s, RC(0x0101000000000101ull));
  ROUND(s, RC(0x0100010100010000ull));
  ROUND(s, RC(0x0100010000010001ull));
  ROUND(s, RC(0x0100000100010100ull));
  ROUND(s, RC(0x0100000000010101ull));
  ROUND(s, RC(0x0001010101000000ull));
  ROUND(s, RC(0x0001010001000001ull));
  ROUND(s, RC(0x0001000101000100ull));
  ROUND(s, RC(0x0001000001000101ull));
}

forceinline void P8ROUNDS(state_t* s) {
  ROUND(s, RC(0x0100010100010000ull));
  ROUND(s, RC(0x0100010000010001ull));
  ROUND(s, RC(0x0100000100010100ull));
  ROUND(s, RC(0x0100000000010101ull));
  ROUND(s, RC(0x0001010101000000ull));
  ROUND(s, RC(0x0001010001000001ull));
  ROUND(s, RC(0x0001000101000100ull));
  ROUND(s, RC(0x0001000001000101ull));
}

forceinline void P6ROUNDS(state_t* s) {
  ROUND(s, RC(0x0100000100010100ull));
  ROUND(s, RC(0x0100000000010101ull));
  ROUND(s, RC(0x0001010101000000ull));
  ROUND(s, RC(0x0001010001000001ull));
  ROUND(s, RC(0x0001000101000100ull));
  ROUND(s, RC(0x0001000001000101ull));
}

extern const uint64_t constants[12];

forceinline void PROUNDS(state_t* s, int nr) {
  for (int i = START(nr); i < 12; i++) ROUND(s, RC(constants[i]));
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
