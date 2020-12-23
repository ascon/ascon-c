#ifndef ROUND_H_
#define ROUND_H_

#include "ascon.h"
#include "printstate.h"

forceinline void KINIT(word_t* K0, word_t* K1, word_t* K2) {
  *K0 = WORD_T(0);
  *K1 = WORD_T(0);
  *K2 = WORD_T(0);
}

forceinline void PINIT(state_t* s) {
  s->x0 = WORD_T(0);
  s->x1 = WORD_T(0);
  s->x2 = WORD_T(0);
  s->x3 = WORD_T(0);
  s->x4 = WORD_T(0);
}

forceinline void ROUND(state_t* s, word_t C) {
  uint64_t x = 0;
  __mmask8 mxor1 = 0x15;
  __mmask8 mxor2 = 0x0b;
  __m512i pxor1 = _mm512_set_epi64(x, x, x, 3, x, 1, x, 4);
  __m512i pxor2 = _mm512_set_epi64(x, x, x, x, 2, x, 0, 4);
  __m512i c = _mm512_set_epi64(x, x, x, 0, 0, C, 0, 0);
  __m512i n = _mm512_set_epi64(x, x, x, 0, 0, ~0ull, 0, 0);
  __m512i pchi1 = _mm512_set_epi64(x, x, x, 0, 4, 3, 2, 1);
  __m512i pchi2 = _mm512_set_epi64(x, x, x, 1, 0, 4, 3, 2);
  __m512i rot1 = _mm512_set_epi64(x, x, x, 7, 10, 1, 61, 19);
  __m512i rot2 = _mm512_set_epi64(x, x, x, 41, 17, 6, 39, 28);
  __m512i t0, t1, t2;
  /* round constant + s-box layer */
  t0 = _mm512_maskz_permutexvar_epi64(mxor1, pxor1, s->z);
  t0 = _mm512_ternarylogic_epi64(s->z, t0, c, 0x96);
  /* keccak s-box start */
  t1 = _mm512_permutexvar_epi64(pchi1, t0);
  t2 = _mm512_permutexvar_epi64(pchi2, t0);
  t0 = _mm512_ternarylogic_epi64(t0, t1, t2, 0xd2);
  /* keccak s-box end */
  t1 = _mm512_maskz_permutexvar_epi64(mxor2, pxor2, t0);
  t0 = _mm512_ternarylogic_epi64(t0, t1, n, 0x96);
  /* linear layer */
  t1 = _mm512_rorv_epi64(t0, rot1);
  t2 = _mm512_rorv_epi64(t0, rot2);
  s->z = _mm512_ternarylogic_epi64(t0, t1, t2, 0x96);
  printstate(" round output", s);
}

#endif /* ROUND_H_ */
