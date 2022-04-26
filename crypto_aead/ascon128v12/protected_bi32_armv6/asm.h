#ifndef ASM_H_
#define ASM_H_

#ifndef __GNUC__
#define __asm__ asm
#endif

#define LDR(rd, ptr, offset) \
  __asm__ volatile("ldr %0, [%1, %2]\n\t" : "=r"(rd) : "r"(ptr), "ri"(offset))

#define STR(rd, ptr, offset)                                                \
  __asm__ volatile("str %0, [%1, %2]\n\t" ::"r"(rd), "r"(ptr), "ri"(offset) \
                   : "memory")

#define CLEAR()                                            \
  do {                                                     \
    uint32_t r, v = 0;                                     \
    __asm__ volatile("mov %0, %1\n\t" : "=r"(r) : "i"(v)); \
  } while (0)

#define MOV(rd, imm) __asm__ volatile("mov %0, %1\n\t" : "=r"(rd) : "i"(imm))

#define ROR(rd, rn, imm) \
  __asm__ volatile("ror %0, %1, #%c2\n\t" : "=r"(rd) : "r"(rn), "i"(imm))

#define EOR_ROR(rd, rn, rm, imm)                  \
  __asm__ volatile("eor %0, %1, %2, ror #%c3\n\t" \
                   : "=r"(rd)                     \
                   : "r"(rn), "r"(rm), "i"(imm))

#define EOR_AND_ROR(ce, ae, be, imm, tmp)                 \
  __asm__ volatile(                                       \
      "and %[tmp_], %[ae_], %[be_], ror %[i1_]\n\t"       \
      "eor %[ce_], %[tmp_], %[ce_]\n\t"                   \
      : [ce_] "+r"(ce), [tmp_] "=r"(tmp)                  \
      : [ae_] "r"(ae), [be_] "r"(be), [i1_] "i"(ROT(imm)) \
      :)

#define EOR_BIC_ROR(ce, ae, be, imm, tmp)                 \
  __asm__ volatile(                                       \
      "bic %[tmp_], %[ae_], %[be_], ror %[i1_]\n\t"       \
      "eor %[ce_], %[tmp_], %[ce_]\n\t"                   \
      : [ce_] "+r"(ce), [tmp_] "=r"(tmp)                  \
      : [ae_] "r"(ae), [be_] "r"(be), [i1_] "i"(ROT(imm)) \
      :)

#define EOR_ORR_ROR(ce, ae, be, imm, tmp)                 \
  __asm__ volatile(                                       \
      "orr %[tmp_], %[ae_], %[be_], ror %[i1_]\n\t"       \
      "eor %[ce_], %[tmp_], %[ce_]\n\t"                   \
      : [ce_] "+r"(ce), [tmp_] "=r"(tmp)                  \
      : [ae_] "r"(ae), [be_] "r"(be), [i1_] "i"(ROT(imm)) \
      :)

#endif  // ASM_H_
