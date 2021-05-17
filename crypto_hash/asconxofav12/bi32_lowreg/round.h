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
  word_t xtemp;
  /* round constant */
  s->x2 = XOR(s->x2, C);
  /* s-box layer */
  s->x0 = XOR(s->x0, s->x4);
  s->x4 = XOR(s->x4, s->x3);
  s->x2 = XOR(s->x2, s->x1);
  xtemp = AND(s->x0, NOT(s->x4));
  s->x0 = XOR(s->x0, AND(s->x2, NOT(s->x1)));
  s->x2 = XOR(s->x2, AND(s->x4, NOT(s->x3)));
  s->x4 = XOR(s->x4, AND(s->x1, NOT(s->x0)));
  s->x1 = XOR(s->x1, AND(s->x3, NOT(s->x2)));
  s->x3 = XOR(s->x3, xtemp);
  s->x1 = XOR(s->x1, s->x0);
  s->x3 = XOR(s->x3, s->x2);
  s->x0 = XOR(s->x0, s->x4);
  /* linear layer */
  xtemp = XOR(s->x0, ROR(s->x0, 28 - 19));
  s->x0 = XOR(s->x0, ROR(xtemp, 19));
  xtemp = XOR(s->x1, ROR(s->x1, 61 - 39));
  s->x1 = XOR(s->x1, ROR(xtemp, 39));
  xtemp = XOR(s->x2, ROR(s->x2, 6 - 1));
  s->x2 = XOR(s->x2, ROR(xtemp, 1));
  xtemp = XOR(s->x3, ROR(s->x3, 17 - 10));
  s->x3 = XOR(s->x3, ROR(xtemp, 10));
  xtemp = XOR(s->x4, ROR(s->x4, 41 - 7));
  s->x4 = XOR(s->x4, ROR(xtemp, 7));
  s->x2 = NOT(s->x2);
  printstate(" round output", s);
}

#endif /* ROUND_H_ */
