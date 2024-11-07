#ifndef ROUND_H_
#define ROUND_H_

#include "ascon.h"
#include "constants.h"
#include "printstate.h"

forceinline ascon_state_t AFFINE1(ascon_state_t s, int i, int d) {
  s.x[2].s[d].w[i] ^= s.x[1].s[d].w[i];
  s.x[0].s[d].w[i] ^= s.x[4].s[d].w[i];
  s.x[4].s[d].w[i] ^= s.x[3].s[d].w[i];
  return s;
}

forceinline ascon_state_t AFFINE2(ascon_state_t s, int i, int d) {
  s.x[2].s[d].w[i] ^= s.x[5].s[d].w[i];
  s.x[1].s[d].w[i] ^= s.x[0].s[d].w[i];
  s.x[0].s[d].w[i] ^= s.x[4].s[d].w[i];
  s.x[3].s[d].w[i] ^= s.x[2].s[d].w[i];
  return s;
}

forceinline ascon_state_t SBOX(ascon_state_t s, int i, int ns) {
  /* affine layer 1 */
  if (ns >= 1) s = AFFINE1(s, i, 0);
  if (ns >= 2) s = AFFINE1(s, i, 1);
  if (ns >= 3) s = AFFINE1(s, i, 2);
  if (ns >= 4) s = AFFINE1(s, i, 3);
  /* Toffoli gates */
  s.x[5] = MXORBIC(s.x[5], s.x[4], s.x[3], i, ns);
  s.x[4] = MXORBIC(s.x[4], s.x[1], s.x[0], i, ns);
  s.x[1] = MXORBIC(s.x[1], s.x[3], s.x[2], i, ns);
  s.x[3] = MXORBIC(s.x[3], s.x[0], s.x[4], i, ns);
  s.x[0] = MXORBIC(s.x[0], s.x[2], s.x[1], i, ns);
  /* affine layer 2 */
  if (ns >= 1) s = AFFINE2(s, i, 0);
  s.x[2].s[0].w[i] = ~s.x[2].s[0].w[i];
  if (ns >= 2) s = AFFINE2(s, i, 1);
  if (ns >= 3) s = AFFINE2(s, i, 2);
  if (ns >= 4) s = AFFINE2(s, i, 3);
  return s;
}

forceinline ascon_state_t LINEAR(ascon_state_t s, int d) {
  ascon_state_t t;
  t.x[0].s[d].w[0] = s.x[0].s[d].w[0] ^ ROR32(s.x[0].s[d].w[1], 4);
  t.x[0].s[d].w[1] = s.x[0].s[d].w[1] ^ ROR32(s.x[0].s[d].w[0], 5);
  t.x[1].s[d].w[0] = s.x[1].s[d].w[0] ^ ROR32(s.x[1].s[d].w[0], 11);
  t.x[1].s[d].w[1] = s.x[1].s[d].w[1] ^ ROR32(s.x[1].s[d].w[1], 11);
  t.x[2].s[d].w[0] = s.x[2].s[d].w[0] ^ ROR32(s.x[2].s[d].w[1], 2);
  t.x[2].s[d].w[1] = s.x[2].s[d].w[1] ^ ROR32(s.x[2].s[d].w[0], 3);
  t.x[3].s[d].w[0] = s.x[3].s[d].w[0] ^ ROR32(s.x[3].s[d].w[1], 3);
  t.x[3].s[d].w[1] = s.x[3].s[d].w[1] ^ ROR32(s.x[3].s[d].w[0], 4);
  t.x[4].s[d].w[0] = s.x[4].s[d].w[0] ^ ROR32(s.x[4].s[d].w[0], 17);
  t.x[4].s[d].w[1] = s.x[4].s[d].w[1] ^ ROR32(s.x[4].s[d].w[1], 17);
  s.x[0].s[d].w[0] ^= ROR32(t.x[0].s[d].w[1], 9);
  s.x[0].s[d].w[1] ^= ROR32(t.x[0].s[d].w[0], 10);
  s.x[1].s[d].w[0] ^= ROR32(t.x[1].s[d].w[1], 19);
  s.x[1].s[d].w[1] ^= ROR32(t.x[1].s[d].w[0], 20);
  s.x[2].s[d].w[0] ^= ROR32(t.x[2].s[d].w[1], 0);
  s.x[2].s[d].w[1] ^= ROR32(t.x[2].s[d].w[0], 1);
  s.x[3].s[d].w[0] ^= ROR32(t.x[3].s[d].w[0], 5);
  s.x[3].s[d].w[1] ^= ROR32(t.x[3].s[d].w[1], 5);
  s.x[4].s[d].w[0] ^= ROR32(t.x[4].s[d].w[1], 3);
  s.x[4].s[d].w[1] ^= ROR32(t.x[4].s[d].w[0], 4);
  return s;
}

forceinline void ROUND_(ascon_state_t* p, uint8_t C_o, uint8_t C_e, int ns) {
  ascon_state_t s = *p;
  /* constant and sbox layer*/
  s.x[2].s[0].w[0] ^= C_e;
  s = SBOX(s, 0, ns);
  s.x[2].s[0].w[1] ^= C_o;
  s = SBOX(s, 1, ns);
  /* reuse rotated randomness */
  s.x[5] = MREUSE(s.x[5], 0, ns);
  /* linear layer*/
  if (ns >= 4) s = LINEAR(s, 3);
  if (ns >= 3) s = LINEAR(s, 2);
  if (ns >= 2) s = LINEAR(s, 1);
  if (ns >= 1) s = LINEAR(s, 0);
  *p = s;
  printstate(" round output", &s, ns);
}

forceinline void ROUND(ascon_state_t* p, uint64_t C, int ns) {
  ROUND_(p, C >> 32, C, ns);
}

forceinline void PROUNDS(ascon_state_t* s, int nr, int ns) {
  int i = START(nr);
  do {
    ROUND_(s, RC(i), ns);
    i += INC;
  } while (i != END);
}

#endif /* ROUND_H_ */
