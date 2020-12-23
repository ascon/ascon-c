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

/* clang-format off */
#define ROUND(OFFSET)                   \
  "vldr d31, [%[C], #" #OFFSET "] \n\t" \
  "veor d0, d0, d4 \n\t"                \
  "veor d4, d4, d3 \n\t"                \
  "veor d2, d2, d31 \n\t"               \
  "vbic d13, d0, d4 \n\t"               \
  "vbic d12, d4, d3 \n\t"               \
  "veor d2, d2, d1 \n\t"                \
  "vbic d14, d1, d0 \n\t"               \
  "vbic d11, d3, d2 \n\t"               \
  "vbic d10, d2, d1 \n\t"               \
  "veor q0, q0, q5 \n\t"                \
  "veor q1, q1, q6 \n\t"                \
  "veor d4, d4, d14 \n\t"               \
  "veor d1, d1, d0 \n\t"                \
  "veor d3, d3, d2 \n\t"                \
  "veor d0, d0, d4 \n\t"                \
  "vsri.64 d14, d4, #7 \n\t"            \
  "vsri.64 d24, d4, #41 \n\t"           \
  "vsri.64 d11, d1, #39 \n\t"           \
  "vsri.64 d21, d1, #61 \n\t"           \
  "vsri.64 d10, d0, #19 \n\t"           \
  "vsri.64 d20, d0, #28 \n\t"           \
  "vsri.64 d12, d2, #1 \n\t"            \
  "vsri.64 d22, d2, #6 \n\t"            \
  "vsri.64 d13, d3, #10 \n\t"           \
  "vsri.64 d23, d3, #17 \n\t"           \
  "vsli.64 d10, d0, #45 \n\t"           \
  "vsli.64 d20, d0, #36 \n\t"           \
  "vsli.64 d11, d1, #25 \n\t"           \
  "vsli.64 d21, d1, #3 \n\t"            \
  "vsli.64 d12, d2, #63 \n\t"           \
  "vsli.64 d22, d2, #58 \n\t"           \
  "vsli.64 d13, d3, #54 \n\t"           \
  "vsli.64 d23, d3, #47 \n\t"           \
  "vsli.64 d14, d4, #57 \n\t"           \
  "vsli.64 d24, d4, #23 \n\t"           \
  "veor q5, q5, q0 \n\t"                \
  "veor q6, q6, q1 \n\t"                \
  "veor d14, d14, d4 \n\t"              \
  "veor q0, q5, q10 \n\t"               \
  "veor d4, d14, d24 \n\t"              \
  "veor q1, q6, q11 \n\t"
/* clang-format on */

#endif /* ROUND_H_ */
