#ifndef ROUND_H_
#define ROUND_H_

#include "ascon.h"
#include "constants.h"
#include "forceinline.h"
#include "printstate.h"
#include "word.h"

forceinline void LINEAR_LAYER(ascon_state_t* s, uint64_t xtemp) {
  uint64_t temp;
  temp = s->x[2] ^ ROR(s->x[2], 28 - 19);
  s->x[0] = s->x[2] ^ ROR(temp, 19);
  temp = s->x[4] ^ ROR(s->x[4], 6 - 1);
  s->x[2] = s->x[4] ^ ROR(temp, 1);
  temp = s->x[1] ^ ROR(s->x[1], 41 - 7);
  s->x[4] = s->x[1] ^ ROR(temp, 7);
  temp = s->x[3] ^ ROR(s->x[3], 61 - 39);
  s->x[1] = s->x[3] ^ ROR(temp, 39);
  temp = xtemp ^ ROR(xtemp, 17 - 10);
  s->x[3] = xtemp ^ ROR(temp, 10);
}

forceinline void NONLINEAR_LAYER(ascon_state_t* s, word_t* xtemp, uint8_t pos) {
  uint8_t t0;
  uint8_t t1;
  uint8_t t2;
  // Based on the round description of Ascon given in the Bachelor's thesis:
  //"Optimizing Ascon on RISC-V" of Lars Jellema
  // see https://github.com/Lucus16/ascon-riscv/
  t0 = XOR8(s->b[1][pos], s->b[2][pos]);
  t1 = XOR8(s->b[0][pos], s->b[4][pos]);
  t2 = XOR8(s->b[3][pos], s->b[4][pos]);
  s->b[4][pos] = OR8(s->b[3][pos], NOT8(s->b[4][pos]));
  s->b[4][pos] = XOR8(s->b[4][pos], t0);
  s->b[3][pos] = XOR8(s->b[3][pos], s->b[1][pos]);
  s->b[3][pos] = OR8(s->b[3][pos], t0);
  s->b[3][pos] = XOR8(s->b[3][pos], t1);
  s->b[2][pos] = XOR8(s->b[2][pos], t1);
  s->b[2][pos] = OR8(s->b[2][pos], s->b[1][pos]);
  s->b[2][pos] = XOR8(s->b[2][pos], t2);
  s->b[1][pos] = AND8(s->b[1][pos], NOT8(t1));
  s->b[1][pos] = XOR8(s->b[1][pos], t2);
  s->b[0][pos] = OR8(s->b[0][pos], t2);
  (*xtemp).b[pos] = XOR8(s->b[0][pos], t0);
}

forceinline void ROUND(ascon_state_t* s, uint8_t C) {
  int i;
  word_t xtemp;
  /* round constant */
  s->b[2][0] = XOR8(s->b[2][0], C);
  /* s-box layer */
  for (i = 0; i < 8; i++) NONLINEAR_LAYER(s, &xtemp, i);
  /* linear layer */
  LINEAR_LAYER(s, xtemp.x);
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
