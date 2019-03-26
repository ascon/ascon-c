#ifndef PERMUTATIONS_H_
#define PERMUTATIONS_H_

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef struct {
  u64 x0, x1, x2, x3, x4;
} state;

static const u64 C[12] = {
  0xffffffffffffff0full,
  0xffffffffffffff1eull,
  0xffffffffffffff2dull,
  0xffffffffffffff3cull,
  0xffffffffffffff4bull,
  0xffffffffffffff5aull,
  0xffffffffffffff69ull,
  0xffffffffffffff78ull,
  0xffffffffffffff87ull,
  0xffffffffffffff96ull,
  0xffffffffffffffa5ull,
  0xffffffffffffffb4ull,
};

#define EXT_BYTE64(x, n) ((u8)((u64)(x) >> (8 * (7 - (n)))))
#define INS_BYTE64(x, n) ((u64)(x) << (8 * (7 - (n))))

#define ROUND(OFFSET) \
  "vldr d31, [%[C], #" #OFFSET "] \n\t" \
  "veor d0, d0, d4 \n\t" \
  "veor d4, d4, d3 \n\t" \
  "veor d2, d2, d31 \n\t" \
  "vbic d13, d0, d4 \n\t" \
  "vbic d12, d4, d3 \n\t" \
  "veor d2, d2, d1 \n\t" \
  "vbic d14, d1, d0 \n\t" \
  "vbic d11, d3, d2 \n\t" \
  "vbic d10, d2, d1 \n\t" \
  "veor q0, q0, q5 \n\t" \
  "veor q1, q1, q6 \n\t" \
  "veor d4, d4, d14 \n\t" \
  "veor d1, d1, d0 \n\t" \
  "veor d3, d3, d2 \n\t" \
  "veor d0, d0, d4 \n\t" \
  "vsri.64 d14, d4, #7 \n\t" \
  "vsri.64 d24, d4, #41 \n\t" \
  "vsri.64 d11, d1, #39 \n\t" \
  "vsri.64 d21, d1, #61 \n\t" \
  "vsri.64 d10, d0, #19 \n\t" \
  "vsri.64 d20, d0, #28 \n\t" \
  "vsri.64 d12, d2, #1 \n\t" \
  "vsri.64 d22, d2, #6 \n\t" \
  "vsri.64 d13, d3, #10 \n\t" \
  "vsri.64 d23, d3, #17 \n\t" \
  "vsli.64 d10, d0, #45 \n\t" \
  "vsli.64 d20, d0, #36 \n\t" \
  "vsli.64 d11, d1, #25 \n\t" \
  "vsli.64 d21, d1, #3 \n\t" \
  "vsli.64 d12, d2, #63 \n\t" \
  "vsli.64 d22, d2, #58 \n\t" \
  "vsli.64 d13, d3, #54 \n\t" \
  "vsli.64 d23, d3, #47 \n\t" \
  "vsli.64 d14, d4, #57 \n\t" \
  "vsli.64 d24, d4, #23 \n\t" \
  "veor q5, q5, q0 \n\t" \
  "veor q6, q6, q1 \n\t" \
  "veor d14, d14, d4 \n\t" \
  "veor q0, q5, q10 \n\t" \
  "veor d4, d14, d24 \n\t" \
  "veor q1, q6, q11 \n\t"

#define P12() \
  __asm__ __volatile__ ( \
      ".arm \n\t" \
      ".fpu neon \n\t" \
      "vldm %[s], {d0-d4} \n\t" \
      "vmvn d2, d2 \n\t" \
      ROUND(0) \
      ROUND(8) \
      ROUND(16) \
      ROUND(24) \
      ROUND(32) \
      ROUND(40) \
      ROUND(48) \
      ROUND(56) \
      ROUND(64) \
      ROUND(72) \
      ROUND(80) \
      ROUND(88) \
      "vmvn d2, d2 \n\t" \
      "vstm %[s], {d0-d4} \n\t" \
      :: [s] "r" (&s), [C] "r" (C) \
      : "d0", "d1", "d2", "d3", "d4", \
        "d10", "d11", "d12", "d13", "d14", \
        "d20", "d21", "d22", "d23", "d24", \
        "d31", "memory")

#define P8() \
  __asm__ __volatile__ ( \
      ".arm \n\t" \
      ".fpu neon \n\t" \
      "vldm %[s], {d0-d4} \n\t" \
      "vmvn d2, d2 \n\t" \
      ROUND(32) \
      ROUND(40) \
      ROUND(48) \
      ROUND(56) \
      ROUND(64) \
      ROUND(72) \
      ROUND(80) \
      ROUND(88) \
      "vmvn d2, d2 \n\t" \
      "vstm %[s], {d0-d4} \n\t" \
      :: [s] "r" (&s), [C] "r" (C) \
      : "d0", "d1", "d2", "d3", "d4", \
        "d10", "d11", "d12", "d13", "d14", \
        "d20", "d21", "d22", "d23", "d24", \
        "d31", "memory")

#define AD() \
  do { \
    u32 adlen_hi = (u32)(adlen >> 32); \
    u32 adlen_lo = (u32)adlen; \
    __asm__ __volatile__ ( \
        ".arm \n\t" \
        ".fpu neon \n\t" \
        "cmp %[adlen_hi], #0 \n\t" \
        "cmpeq %[adlen_lo], #15 \n\t" \
        "bls .LAD1 \n\t" \
        "vldm %[s], {d0-d4} \n\t" \
        ".LAD0: \n\t" \
        "vldm %[ad]!, {d16,d17} \n\t" \
        "vrev64.8 q8, q8 \n\t" \
        "veor q0, q0, q8 \n\t" \
        "vmvn d2, d2 \n\t" \
        ROUND(32) \
        ROUND(40) \
        ROUND(48) \
        ROUND(56) \
        ROUND(64) \
        ROUND(72) \
        ROUND(80) \
        ROUND(88) \
        "vmvn d2, d2 \n\t" \
        "subs %[adlen_lo], %[adlen_lo], #16 \n\t" \
        "sbc %[adlen_hi], %[adlen_hi], #0 \n\t" \
        "cmp %[adlen_hi], #0 \n\t" \
        "cmpeq %[adlen_lo], #15 \n\t" \
        "bhi .LAD0 \n\t" \
        "vstm %[s], {d0-d4} \n\t" \
        ".LAD1: \n\t" \
        : [adlen_hi] "+r" (adlen_hi), [adlen_lo] "+r" (adlen_lo), \
          [ad] "+r" (ad) \
        : [s] "r" (&s), [C] "r" (C) \
        : "d0", "d1", "d2", "d3", "d4", \
          "d10", "d11", "d12", "d13", "d14", "d16", "d17", \
          "d20", "d21", "d22", "d23", "d24", \
          "d31", "memory"); \
    adlen = (u64)adlen_hi << 32 | adlen_lo; \
  } while (0)

#define PT() \
  do { \
    u32 mlen_hi = (u32)(mlen >> 32); \
    u32 mlen_lo = (u32)mlen; \
    __asm__ __volatile__ ( \
        ".arm \n\t" \
        ".fpu neon \n\t" \
        "cmp %[mlen_hi], #0 \n\t" \
        "cmpeq %[mlen_lo], #15 \n\t" \
        "bls .LPT1 \n\t" \
        "vldm %[s], {d0-d4} \n\t" \
        ".LPT0: \n\t" \
        "vldm %[m]!, {d16,d17} \n\t" \
        "vrev64.8 q8, q8 \n\t" \
        "veor q0, q0, q8 \n\t" \
        "vrev64.8 q13, q0 \n\t" \
        "vstm %[c]!, {d26,d27} \n\t" \
        "vmvn d2, d2 \n\t" \
        ROUND(32) \
        ROUND(40) \
        ROUND(48) \
        ROUND(56) \
        ROUND(64) \
        ROUND(72) \
        ROUND(80) \
        ROUND(88) \
        "vmvn d2, d2 \n\t" \
        "subs %[mlen_lo], %[mlen_lo], #16 \n\t" \
        "sbc %[mlen_hi], %[mlen_hi], #0 \n\t" \
        "cmp %[mlen_hi], #0 \n\t" \
        "cmpeq %[mlen_lo], #15 \n\t" \
        "bhi .LPT0 \n\t" \
        "vstm %[s], {d0-d4} \n\t" \
        ".LPT1: \n\t" \
        : [mlen_hi] "+r" (mlen_hi), [mlen_lo] "+r" (mlen_lo), \
          [m] "+r" (m), [c] "+r" (c) \
        : [s] "r" (&s), [C] "r" (C) \
        : "d0", "d1", "d2", "d3", "d4", \
          "d10", "d11", "d12", "d13", "d14", "d16", "d17", \
          "d20", "d21", "d22", "d23", "d24", "d26", "d27", \
          "d31", "memory"); \
    mlen = (u64)mlen_hi << 32 | mlen_lo; \
  } while (0)

#define CT() \
  do { \
    u32 clen_hi = (u32)(clen >> 32); \
    u32 clen_lo = (u32)clen; \
    __asm__ __volatile__ ( \
        ".arm \n\t" \
        ".fpu neon \n\t" \
        "cmp %[clen_hi], #0 \n\t" \
        "cmpeq %[clen_lo], #15 \n\t" \
        "bls .LCT1 \n\t" \
        "vldm %[s], {d0-d4} \n\t" \
        ".LCT0: \n\t" \
        "vldm %[c]!, {d26,d27} \n\t" \
        "vrev64.8 q8, q0 \n\t" \
        "veor q8, q8, q13 \n\t" \
        "vrev64.8 q0, q13 \n\t" \
        "vstm %[m]!, {d16,d17} \n\t" \
        "vmvn d2, d2 \n\t" \
        ROUND(32) \
        ROUND(40) \
        ROUND(48) \
        ROUND(56) \
        ROUND(64) \
        ROUND(72) \
        ROUND(80) \
        ROUND(88) \
        "vmvn d2, d2 \n\t" \
        "subs %[clen_lo], %[clen_lo], #16 \n\t" \
        "sbc %[clen_hi], %[clen_hi], #0 \n\t" \
        "cmp %[clen_hi], #0 \n\t" \
        "cmpeq %[clen_lo], #15 \n\t" \
        "bhi .LCT0 \n\t" \
        "vstm %[s], {d0-d4} \n\t" \
        ".LCT1: \n\t" \
        : [clen_hi] "+r" (clen_hi), [clen_lo] "+r" (clen_lo), \
          [m] "+r" (m), [c] "+r" (c) \
        : [s] "r" (&s), [C] "r" (C) \
        : "d0", "d1", "d2", "d3", "d4", \
          "d10", "d11", "d12", "d13", "d14", "d16", "d17", \
          "d20", "d21", "d22", "d23", "d24", "d26", "d27", \
          "d31", "memory"); \
    clen = (u64)clen_hi << 32 | clen_lo; \
  } while (0)

#endif  // PERMUTATIONS_H_

