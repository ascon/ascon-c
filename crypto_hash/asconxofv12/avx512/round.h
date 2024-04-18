#ifndef ROUND_H_
#define ROUND_H_

#include "ascon.h"
#include "constants.h"
#include "forceinline.h"
#include "printstate.h"
#include "word.h"

forceinline void ROUND(ascon_state_t* s, uint8_t C) {
  const uint64_t x = 0;
  const __mmask8 mxor1 = 0x15;
  const __mmask8 mxor2 = 0x0b;
  const __m512i pxor1 = _mm512_set_epi64(x, x, x, 3, x, 1, x, 4);
  const __m512i pxor2 = _mm512_set_epi64(x, x, x, x, 2, x, 0, 4);
  const __m512i rc = _mm512_set_epi64(x, x, x, 0, 0, C, 0, 0);
  const __m512i neg = _mm512_set_epi64(x, x, x, 0, 0, ~0ull, 0, 0);
  const __m512i pchi1 = _mm512_set_epi64(x, x, x, 0, 4, 3, 2, 1);
  const __m512i pchi2 = _mm512_set_epi64(x, x, x, 1, 0, 4, 3, 2);
  const __m512i rot1 = _mm512_set_epi64(x, x, x, 7, 10, 1, 61, 19);
  const __m512i rot2 = _mm512_set_epi64(x, x, x, 41, 17, 6, 39, 28);
  __m512i t0, t1, t2;
  /* round constant + s-box layer */
  t0 = _mm512_maskz_permutexvar_epi64(mxor1, pxor1, s->z);
  t0 = _mm512_ternarylogic_epi64(s->z, t0, rc, 0x96);
  /* keccak s-box start */
  t1 = _mm512_permutexvar_epi64(pchi1, t0);
  t2 = _mm512_permutexvar_epi64(pchi2, t0);
  t0 = _mm512_ternarylogic_epi64(t0, t1, t2, 0xd2);
  /* keccak s-box end */
  t1 = _mm512_maskz_permutexvar_epi64(mxor2, pxor2, t0);
  t0 = _mm512_ternarylogic_epi64(t0, t1, neg, 0x96);
  /* linear layer */
  t1 = _mm512_rorv_epi64(t0, rot1);
  t2 = _mm512_rorv_epi64(t0, rot2);
  s->z = _mm512_ternarylogic_epi64(t0, t1, t2, 0x96);
  printstate(" round output", s);
}

forceinline void PROUNDS(ascon_state_t* s, int nr) {
  int i = START(nr);
  do {
    ROUND(s, RC(i));
    i += INC;
  } while (i != END);
}

#endif /* ROUND_H_ */
