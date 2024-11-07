#ifndef ROUND_H_
#define ROUND_H_

#include "ascon.h"
#include "constants.h"
#include "forceinline.h"
#include "printstate.h"
#include "word.h"

forceinline void ROUND_LOOP(ascon_state_t* s, const uint8_t* C,
                            const uint8_t* E) {
  uint32_t tmp0, tmp1;
  __asm__ __volatile__(
      "rbegin_%=:;\n\t"
      "ldrb %[tmp1], [%[C]], #1\n\t"
      "eor %[x0_l], %[x0_l], %[x4_l]\n\t"
      "eor %[x4_l], %[x4_l], %[x3_l]\n\t"
      "eor %[x2_l], %[x2_l], %[x1_l]\n\t"
      "orn %[tmp0], %[x4_l], %[x0_l]\n\t"
      "eor %[x2_l], %[x2_l], %[tmp1]\n\t"
      "bic %[tmp1], %[x2_l], %[x1_l]\n\t"
      "eor %[x0_l], %[x0_l], %[tmp1]\n\t"
      "orn %[tmp1], %[x3_l], %[x4_l]\n\t"
      "eor %[x2_l], %[x2_l], %[tmp1]\n\t"
      "bic %[tmp1], %[x1_l], %[x0_l]\n\t"
      "eor %[x4_l], %[x4_l], %[tmp1]\n\t"
      "and %[tmp1], %[x3_l], %[x2_l]\n\t"
      "eor %[x1_l], %[x1_l], %[tmp1]\n\t"
      "eor %[x3_l], %[x3_l], %[tmp0]\n\t"
      "ldrb %[tmp1], [%[C]], #1\n\t"
      "eor %[x1_l], %[x1_l], %[x0_l]\n\t"
      "eor %[x3_l], %[x3_l], %[x2_l]\n\t"
      "eor %[x0_l], %[x0_l], %[x4_l]\n\t"
      "eor %[x0_h], %[x0_h], %[x4_h]\n\t"
      "eor %[x4_h], %[x4_h], %[x3_h]\n\t"
      "eor %[x2_h], %[x2_h], %[tmp1]\n\t"
      "eor %[x2_h], %[x2_h], %[x1_h]\n\t"
      "orn %[tmp0], %[x4_h], %[x0_h]\n\t"
      "bic %[tmp1], %[x2_h], %[x1_h]\n\t"
      "eor %[x0_h], %[x0_h], %[tmp1]\n\t"
      "orn %[tmp1], %[x3_h], %[x4_h]\n\t"
      "eor %[x2_h], %[x2_h], %[tmp1]\n\t"
      "bic %[tmp1], %[x1_h], %[x0_h]\n\t"
      "eor %[x4_h], %[x4_h], %[tmp1]\n\t"
      "and %[tmp1], %[x3_h], %[x2_h]\n\t"
      "eor %[x1_h], %[x1_h], %[tmp1]\n\t"
      "eor %[x3_h], %[x3_h], %[tmp0]\n\t"
      "eor %[x1_h], %[x1_h], %[x0_h]\n\t"
      "eor %[x3_h], %[x3_h], %[x2_h]\n\t"
      "eor %[x0_h], %[x0_h], %[x4_h]\n\t"
      "eor %[tmp0], %[x0_l], %[x0_h], ror #4\n\t"
      "eor %[tmp1], %[x0_h], %[x0_l], ror #5\n\t"
      "eor %[x0_h], %[x0_h], %[tmp0], ror #10\n\t"
      "eor %[x0_l], %[x0_l], %[tmp1], ror #9\n\t"
      "eor %[tmp0], %[x1_l], %[x1_l], ror #11\n\t"
      "eor %[tmp1], %[x1_h], %[x1_h], ror #11\n\t"
      "eor %[x1_h], %[x1_h], %[tmp0], ror #20\n\t"
      "eor %[x1_l], %[x1_l], %[tmp1], ror #19\n\t"
      "eor %[tmp0], %[x2_l], %[x2_h], ror #2\n\t"
      "eor %[tmp1], %[x2_h], %[x2_l], ror #3\n\t"
      "eor %[x2_h], %[x2_h], %[tmp0], ror #1\n\t"
      "eor %[x2_l], %[x2_l], %[tmp1]\n\t"
      "eor %[tmp0], %[x3_l], %[x3_h], ror #3\n\t"
      "eor %[tmp1], %[x3_h], %[x3_l], ror #4\n\t"
      "eor %[x3_l], %[x3_l], %[tmp0], ror #5\n\t"
      "eor %[x3_h], %[x3_h], %[tmp1], ror #5\n\t"
      "eor %[tmp0], %[x4_l], %[x4_l], ror #17\n\t"
      "eor %[tmp1], %[x4_h], %[x4_h], ror #17\n\t"
      "eor %[x4_h], %[x4_h], %[tmp0], ror #4\n\t"
      "eor %[x4_l], %[x4_l], %[tmp1], ror #3\n\t"
      "cmp %[C], %[E]\n\t"
      "bne rbegin_%=\n\t"
      :
      [x0_l] "+r"(s->w[0][0]), [x0_h] "+r"(s->w[0][1]), [x1_l] "+r"(s->w[1][0]),
      [x1_h] "+r"(s->w[1][1]), [x2_l] "+r"(s->w[2][0]), [x2_h] "+r"(s->w[2][1]),
      [x3_l] "+r"(s->w[3][0]), [x3_h] "+r"(s->w[3][1]), [x4_l] "+r"(s->w[4][0]),
      [x4_h] "+r"(s->w[4][1]), [C] "+r"(C), [E] "+r"(E), [tmp0] "=r"(tmp0),
      [tmp1] "=r"(tmp1)
      :
      : "cc");
}

forceinline void ROUND(ascon_state_t* s, uint64_t C) {
  uint32_t tmp0, tmp1;
  __asm__ __volatile__(
      "eor %[x0_l], %[x0_l], %[x4_l]\n\t"
      "eor %[x4_l], %[x4_l], %[x3_l]\n\t"
      "eor %[x2_l], %[x2_l], %[x1_l]\n\t"
      "orn %[tmp0], %[x4_l], %[x0_l]\n\t"
      "eor %[x2_l], %[x2_l], %[C_e]\n\t"
      "bic %[tmp1], %[x2_l], %[x1_l]\n\t"
      "eor %[x0_l], %[x0_l], %[tmp1]\n\t"
      "orn %[tmp1], %[x3_l], %[x4_l]\n\t"
      "eor %[x2_l], %[x2_l], %[tmp1]\n\t"
      "bic %[tmp1], %[x1_l], %[x0_l]\n\t"
      "eor %[x4_l], %[x4_l], %[tmp1]\n\t"
      "and %[tmp1], %[x3_l], %[x2_l]\n\t"
      "eor %[x1_l], %[x1_l], %[tmp1]\n\t"
      "eor %[x3_l], %[x3_l], %[tmp0]\n\t"
      "eor %[x1_l], %[x1_l], %[x0_l]\n\t"
      "eor %[x3_l], %[x3_l], %[x2_l]\n\t"
      "eor %[x0_l], %[x0_l], %[x4_l]\n\t"
      "eor %[x0_h], %[x0_h], %[x4_h]\n\t"
      "eor %[x4_h], %[x4_h], %[x3_h]\n\t"
      "eor %[x2_h], %[x2_h], %[C_o]\n\t"
      "eor %[x2_h], %[x2_h], %[x1_h]\n\t"
      "orn %[tmp0], %[x4_h], %[x0_h]\n\t"
      "bic %[tmp1], %[x2_h], %[x1_h]\n\t"
      "eor %[x0_h], %[x0_h], %[tmp1]\n\t"
      "orn %[tmp1], %[x3_h], %[x4_h]\n\t"
      "eor %[x2_h], %[x2_h], %[tmp1]\n\t"
      "bic %[tmp1], %[x1_h], %[x0_h]\n\t"
      "eor %[x4_h], %[x4_h], %[tmp1]\n\t"
      "and %[tmp1], %[x3_h], %[x2_h]\n\t"
      "eor %[x1_h], %[x1_h], %[tmp1]\n\t"
      "eor %[x3_h], %[x3_h], %[tmp0]\n\t"
      "eor %[x1_h], %[x1_h], %[x0_h]\n\t"
      "eor %[x3_h], %[x3_h], %[x2_h]\n\t"
      "eor %[x0_h], %[x0_h], %[x4_h]\n\t"
      "eor %[tmp0], %[x0_l], %[x0_h], ror #4\n\t"
      "eor %[tmp1], %[x0_h], %[x0_l], ror #5\n\t"
      "eor %[x0_h], %[x0_h], %[tmp0], ror #10\n\t"
      "eor %[x0_l], %[x0_l], %[tmp1], ror #9\n\t"
      "eor %[tmp0], %[x1_l], %[x1_l], ror #11\n\t"
      "eor %[tmp1], %[x1_h], %[x1_h], ror #11\n\t"
      "eor %[x1_h], %[x1_h], %[tmp0], ror #20\n\t"
      "eor %[x1_l], %[x1_l], %[tmp1], ror #19\n\t"
      "eor %[tmp0], %[x2_l], %[x2_h], ror #2\n\t"
      "eor %[tmp1], %[x2_h], %[x2_l], ror #3\n\t"
      "eor %[x2_h], %[x2_h], %[tmp0], ror #1\n\t"
      "eor %[x2_l], %[x2_l], %[tmp1]\n\t"
      "eor %[tmp0], %[x3_l], %[x3_h], ror #3\n\t"
      "eor %[tmp1], %[x3_h], %[x3_l], ror #4\n\t"
      "eor %[x3_l], %[x3_l], %[tmp0], ror #5\n\t"
      "eor %[x3_h], %[x3_h], %[tmp1], ror #5\n\t"
      "eor %[tmp0], %[x4_l], %[x4_l], ror #17\n\t"
      "eor %[tmp1], %[x4_h], %[x4_h], ror #17\n\t"
      "eor %[x4_h], %[x4_h], %[tmp0], ror #4\n\t"
      "eor %[x4_l], %[x4_l], %[tmp1], ror #3\n\t"
      :
      [x0_l] "+r"(s->w[0][0]), [x0_h] "+r"(s->w[0][1]), [x1_l] "+r"(s->w[1][0]),
      [x1_h] "+r"(s->w[1][1]), [x2_l] "+r"(s->w[2][0]), [x2_h] "+r"(s->w[2][1]),
      [x3_l] "+r"(s->w[3][0]), [x3_h] "+r"(s->w[3][1]), [x4_l] "+r"(s->w[4][0]),
      [x4_h] "+r"(s->w[4][1]), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1)
      : [C_e] "i"((uint32_t)C), [C_o] "i"((uint32_t)(C >> 32))
      :);
}

#define ROUND5(x0, x1, x2, x3, x4, C)                                       \
  do {                                                                      \
    uint32_t tmp0, tmp1, tmp2;                                              \
    /* Based on the round description of Ascon given in the Bachelor's */   \
    /* thesis: "Optimizing Ascon on RISC-V" of Lars Jellema */              \
    /* see https://github.com/Lucus16/ascon-riscv/ */                       \
    __asm__ __volatile__(                                                   \
        "eor %[x2_l], %[x2_l], %[C_e]\n\t"                                  \
        "eor %[tmp0], %[x1_l], %[x2_l]\n\t"                                 \
        "eor %[tmp1], %[x0_l], %[x4_l]\n\t"                                 \
        "eor %[tmp2], %[x3_l], %[x4_l]\n\t"                                 \
        "orn %[x4_l], %[x3_l], %[x4_l]\n\t"                                 \
        "eor %[x4_l], %[x4_l], %[tmp0]\n\t"                                 \
        "eor %[x3_l], %[x3_l], %[x1_l]\n\t"                                 \
        "orr %[x3_l], %[x3_l], %[tmp0]\n\t"                                 \
        "eor %[x3_l], %[x3_l], %[tmp1]\n\t"                                 \
        "eor %[x2_l], %[x2_l], %[tmp1]\n\t"                                 \
        "orr %[x2_l], %[x2_l], %[x1_l]\n\t"                                 \
        "eor %[x2_l], %[x2_l], %[tmp2]\n\t"                                 \
        "bic %[x1_l], %[x1_l], %[tmp1]\n\t"                                 \
        "eor %[x1_l], %[x1_l], %[tmp2]\n\t"                                 \
        "orr %[x0_l], %[x0_l], %[tmp2]\n\t"                                 \
        "eor %[x0_l], %[x0_l], %[tmp0]\n\t"                                 \
        "eor %[x2_h], %[x2_h], %[C_o]\n\t"                                  \
        "eor %[tmp0], %[x1_h], %[x2_h]\n\t"                                 \
        "eor %[tmp1], %[x0_h], %[x4_h]\n\t"                                 \
        "eor %[tmp2], %[x3_h], %[x4_h]\n\t"                                 \
        "orn %[x4_h], %[x3_h], %[x4_h]\n\t"                                 \
        "eor %[x4_h], %[x4_h], %[tmp0]\n\t"                                 \
        "eor %[x3_h], %[x3_h], %[x1_h]\n\t"                                 \
        "orr %[x3_h], %[x3_h], %[tmp0]\n\t"                                 \
        "eor %[x3_h], %[x3_h], %[tmp1]\n\t"                                 \
        "eor %[x2_h], %[x2_h], %[tmp1]\n\t"                                 \
        "orr %[x2_h], %[x2_h], %[x1_h]\n\t"                                 \
        "eor %[x2_h], %[x2_h], %[tmp2]\n\t"                                 \
        "bic %[x1_h], %[x1_h], %[tmp1]\n\t"                                 \
        "eor %[x1_h], %[x1_h], %[tmp2]\n\t"                                 \
        "orr %[x0_h], %[x0_h], %[tmp2]\n\t"                                 \
        "eor %[x0_h], %[x0_h], %[tmp0]\n\t"                                 \
        "eor %[tmp0], %[x2_l], %[x2_h], ror #4\n\t"                         \
        "eor %[tmp1], %[x2_h], %[x2_l], ror #5\n\t"                         \
        "eor %[x2_h], %[x2_h], %[tmp0], ror #10\n\t"                        \
        "eor %[x2_l], %[x2_l], %[tmp1], ror #9\n\t"                         \
        "eor %[tmp0], %[x3_l], %[x3_l], ror #11\n\t"                        \
        "eor %[tmp1], %[x3_h], %[x3_h], ror #11\n\t"                        \
        "eor %[x3_h], %[x3_h], %[tmp0], ror #20\n\t"                        \
        "eor %[x3_l], %[x3_l], %[tmp1], ror #19\n\t"                        \
        "eor %[tmp0], %[x4_l], %[x4_h], ror #2\n\t"                         \
        "eor %[tmp1], %[x4_h], %[x4_l], ror #3\n\t"                         \
        "eor %[x4_h], %[x4_h], %[tmp0], ror #1\n\t"                         \
        "eor %[x4_l], %[x4_l], %[tmp1]\n\t"                                 \
        "eor %[tmp0], %[x0_l], %[x0_h], ror #3\n\t"                         \
        "eor %[tmp1], %[x0_h], %[x0_l], ror #4\n\t"                         \
        "eor %[x0_l], %[x0_l], %[tmp0], ror #5\n\t"                         \
        "eor %[x0_h], %[x0_h], %[tmp1], ror #5\n\t"                         \
        "eor %[tmp0], %[x1_l], %[x1_l], ror #17\n\t"                        \
        "eor %[tmp1], %[x1_h], %[x1_h], ror #17\n\t"                        \
        "eor %[x1_h], %[x1_h], %[tmp0], ror #4\n\t"                         \
        "eor %[x1_l], %[x1_l], %[tmp1], ror #3\n\t"                         \
        : [x0_l] "+r"(x0.w[0]), [x0_h] "+r"(x0.w[1]), [x1_l] "+r"(x1.w[0]), \
          [x1_h] "+r"(x1.w[1]), [x2_l] "+r"(x2.w[0]), [x2_h] "+r"(x2.w[1]), \
          [x3_l] "+r"(x3.w[0]), [x3_h] "+r"(x3.w[1]), [x4_l] "+r"(x4.w[0]), \
          [x4_h] "+r"(x4.w[1]), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1),       \
          [tmp2] "=r"(tmp2)                                                 \
        : [C_e] "i"((uint32_t)C), [C_o] "i"((uint32_t)(C >> 32))            \
        :);                                                                 \
  } while (0)

forceinline void PROUNDS(ascon_state_t* s, int nr) {
  ROUND_LOOP(s, constants + START(nr), constants + 24);
}

#endif /* ROUND_H_ */
