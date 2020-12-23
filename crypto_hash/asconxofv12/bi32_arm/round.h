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
  uint32_t tmp0, tmp1, tmp2, tmp3;
  /* clang-format off */
  __asm__ __volatile__(                            \
      "eor %[x2_e], %[x2_e], %[C_e]\n\t"           \
      "eor %[x2_o], %[x2_o], %[C_o]\n\t"           \
      "eor %[x0_e], %[x0_e], %[x4_e]\n\t"          \
      "eor %[x0_o], %[x0_o], %[x4_o]\n\t"          \
      "eor %[x4_e], %[x4_e], %[x3_e]\n\t"          \
      "eor %[x4_o], %[x4_o], %[x3_o]\n\t"          \
      "eor %[x2_e], %[x2_e], %[x1_e]\n\t"          \
      "eor %[x2_o], %[x2_o], %[x1_o]\n\t"          \
      "bic %[tmp0], %[x0_e], %[x4_e]\n\t"          \
      "bic %[tmp1], %[x4_e], %[x3_e]\n\t"          \
      "bic %[tmp2], %[x2_e], %[x1_e]\n\t"          \
      "bic %[tmp3], %[x1_e], %[x0_e]\n\t"          \
      "eor %[x2_e], %[x2_e], %[tmp1]\n\t"          \
      "eor %[x0_e], %[x0_e], %[tmp2]\n\t"          \
      "eor %[x4_e], %[x4_e], %[tmp3]\n\t"          \
      "bic %[tmp3], %[x3_e], %[x2_e]\n\t"          \
      "eor %[x3_e], %[x3_e], %[tmp0]\n\t"          \
      "bic %[tmp2], %[x0_o], %[x4_o]\n\t"          \
      "bic %[tmp0], %[x2_o], %[x1_o]\n\t"          \
      "bic %[tmp1], %[x4_o], %[x3_o]\n\t"          \
      "eor %[x1_e], %[x1_e], %[tmp3]\n\t"          \
      "eor %[x0_o], %[x0_o], %[tmp0]\n\t"          \
      "eor %[x2_o], %[x2_o], %[tmp1]\n\t"          \
      "bic %[tmp3], %[x1_o], %[x0_o]\n\t"          \
      "bic %[tmp0], %[x3_o], %[x2_o]\n\t"          \
      "eor %[x3_o], %[x3_o], %[tmp2]\n\t"          \
      "eor %[x3_o], %[x3_o], %[x2_o]\n\t"          \
      "eor %[x4_o], %[x4_o], %[tmp3]\n\t"          \
      "eor %[x1_o], %[x1_o], %[tmp0]\n\t"          \
      "eor %[x3_e], %[x3_e], %[x2_e]\n\t"          \
      "eor %[x1_e], %[x1_e], %[x0_e]\n\t"          \
      "eor %[x1_o], %[x1_o], %[x0_o]\n\t"          \
      "eor %[x0_e], %[x0_e], %[x4_e]\n\t"          \
      "eor %[x0_o], %[x0_o], %[x4_o]\n\t"          \
      "mvn %[x2_e], %[x2_e]\n\t"                   \
      "mvn %[x2_o], %[x2_o]\n\t"                   \
      "eor %[tmp0], %[x0_e], %[x0_o], ror #4\n\t"  \
      "eor %[tmp1], %[x0_o], %[x0_e], ror #5\n\t"  \
      "eor %[tmp2], %[x1_e], %[x1_e], ror #11\n\t" \
      "eor %[tmp3], %[x1_o], %[x1_o], ror #11\n\t" \
      "eor %[x0_e], %[x0_e], %[tmp1], ror #9\n\t"  \
      "eor %[x0_o], %[x0_o], %[tmp0], ror #10\n\t" \
      "eor %[x1_e], %[x1_e], %[tmp3], ror #19\n\t" \
      "eor %[x1_o], %[x1_o], %[tmp2], ror #20\n\t" \
      "eor %[tmp0], %[x2_e], %[x2_o], ror #2\n\t"  \
      "eor %[tmp1], %[x2_o], %[x2_e], ror #3\n\t"  \
      "eor %[tmp2], %[x3_e], %[x3_o], ror #3\n\t"  \
      "eor %[tmp3], %[x3_o], %[x3_e], ror #4\n\t"  \
      "eor %[x2_e], %[x2_e], %[tmp1]\n\t"          \
      "eor %[x2_o], %[x2_o], %[tmp0], ror #1\n\t"  \
      "eor %[x3_e], %[x3_e], %[tmp2], ror #5\n\t"  \
      "eor %[x3_o], %[x3_o], %[tmp3], ror #5\n\t"  \
      "eor %[tmp0], %[x4_e], %[x4_e], ror #17\n\t" \
      "eor %[tmp1], %[x4_o], %[x4_o], ror #17\n\t" \
      "eor %[x4_e], %[x4_e], %[tmp1], ror #3\n\t"  \
      "eor %[x4_o], %[x4_o], %[tmp0], ror #4\n\t"  \
      : [ x0_e ] "+r"(s->x0.e),                    \
        [ x1_e ] "+r"(s->x1.e),                    \
        [ x2_e ] "+r"(s->x2.e),                    \
        [ x3_e ] "+r"(s->x3.e),                    \
        [ x4_e ] "+r"(s->x4.e),                    \
        [ x0_o ] "+r"(s->x0.o),                    \
        [ x1_o ] "+r"(s->x1.o),                    \
        [ x2_o ] "+r"(s->x2.o),                    \
        [ x3_o ] "+r"(s->x3.o),                    \
        [ x4_o ] "+r"(s->x4.o),                    \
        [ tmp0 ] "=r"(tmp0),                       \
        [ tmp1 ] "=r"(tmp1),                       \
        [ tmp2 ] "=r"(tmp2),                       \
        [ tmp3 ] "=r"(tmp3)                        \
      : [ C_e ] "ri"(C.e),                         \
        [ C_o ] "ri"(C.o)                          \
      : );
  /* clang-format on */
  printstate(" round output", s);
}

#endif /* ROUND_H_ */
