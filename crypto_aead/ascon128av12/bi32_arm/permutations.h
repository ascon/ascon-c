#ifndef PERMUTATIONS_H_
#define PERMUTATIONS_H_

#include "endian.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef struct {
  u32 e;
  u32 o;
} u32_2;

#define EXT_BYTE64(x, n) ((u8)((u64)(x) >> (8 * (7 - (n)))))
#define INS_BYTE64(x, n) ((u64)(x) << (8 * (7 - (n))))

// Credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002
#define to_bit_interleaving(out, in)                          \
  do {                                                        \
    u32 hi = (in) >> 32;                                      \
    u32 lo = (u32)(in);                                       \
    u32 r0, r1;                                               \
    r0 = (lo ^ (lo >> 1)) & 0x22222222, lo ^= r0 ^ (r0 << 1); \
    r0 = (lo ^ (lo >> 2)) & 0x0C0C0C0C, lo ^= r0 ^ (r0 << 2); \
    r0 = (lo ^ (lo >> 4)) & 0x00F000F0, lo ^= r0 ^ (r0 << 4); \
    r0 = (lo ^ (lo >> 8)) & 0x0000FF00, lo ^= r0 ^ (r0 << 8); \
    r1 = (hi ^ (hi >> 1)) & 0x22222222, hi ^= r1 ^ (r1 << 1); \
    r1 = (hi ^ (hi >> 2)) & 0x0C0C0C0C, hi ^= r1 ^ (r1 << 2); \
    r1 = (hi ^ (hi >> 4)) & 0x00F000F0, hi ^= r1 ^ (r1 << 4); \
    r1 = (hi ^ (hi >> 8)) & 0x0000FF00, hi ^= r1 ^ (r1 << 8); \
    (out).e = (lo & 0x0000FFFF) | (hi << 16);                 \
    (out).o = (lo >> 16) | (hi & 0xFFFF0000);                 \
  } while (0)

// Credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002
#define from_bit_interleaving(out, in)                        \
  do {                                                        \
    u32 lo = ((in).e & 0x0000FFFF) | ((in).o << 16);          \
    u32 hi = ((in).e >> 16) | ((in).o & 0xFFFF0000);          \
    u32 r0, r1;                                               \
    r0 = (lo ^ (lo >> 8)) & 0x0000FF00, lo ^= r0 ^ (r0 << 8); \
    r0 = (lo ^ (lo >> 4)) & 0x00F000F0, lo ^= r0 ^ (r0 << 4); \
    r0 = (lo ^ (lo >> 2)) & 0x0C0C0C0C, lo ^= r0 ^ (r0 << 2); \
    r0 = (lo ^ (lo >> 1)) & 0x22222222, lo ^= r0 ^ (r0 << 1); \
    r1 = (hi ^ (hi >> 8)) & 0x0000FF00, hi ^= r1 ^ (r1 << 8); \
    r1 = (hi ^ (hi >> 4)) & 0x00F000F0, hi ^= r1 ^ (r1 << 4); \
    r1 = (hi ^ (hi >> 2)) & 0x0C0C0C0C, hi ^= r1 ^ (r1 << 2); \
    r1 = (hi ^ (hi >> 1)) & 0x22222222, hi ^= r1 ^ (r1 << 1); \
    out = (u64)hi << 32 | lo;                                 \
  } while (0)

#define ROUND(C_e, C_o) \
  do { \
    u32 reg0, reg1, reg2, reg3; \
    __asm__ __volatile__ ( \
      "eor %[x2_e], %[x2_e], #" #C_e "\n\t" \
      "eor %[x2_o], %[x2_o], #" #C_o "\n\t" \
      "eor %[x0_e], %[x0_e], %[x4_e]\n\t" \
      "eor %[x0_o], %[x0_o], %[x4_o]\n\t" \
      "eor %[x4_e], %[x4_e], %[x3_e]\n\t" \
      "eor %[x4_o], %[x4_o], %[x3_o]\n\t" \
      "eor %[x2_e], %[x2_e], %[x1_e]\n\t" \
      "eor %[x2_o], %[x2_o], %[x1_o]\n\t" \
      "bic %[reg0], %[x0_e], %[x4_e]\n\t" \
      "bic %[reg1], %[x4_e], %[x3_e]\n\t" \
      "bic %[reg2], %[x2_e], %[x1_e]\n\t" \
      "bic %[reg3], %[x1_e], %[x0_e]\n\t" \
      "eor %[x2_e], %[x2_e], %[reg1]\n\t" \
      "eor %[x0_e], %[x0_e], %[reg2]\n\t" \
      "eor %[x4_e], %[x4_e], %[reg3]\n\t" \
      "bic %[reg3], %[x3_e], %[x2_e]\n\t" \
      "eor %[x3_e], %[x3_e], %[reg0]\n\t" \
      "bic %[reg2], %[x0_o], %[x4_o]\n\t" \
      "bic %[reg0], %[x2_o], %[x1_o]\n\t" \
      "bic %[reg1], %[x4_o], %[x3_o]\n\t" \
      "eor %[x1_e], %[x1_e], %[reg3]\n\t" \
      "eor %[x0_o], %[x0_o], %[reg0]\n\t" \
      "eor %[x2_o], %[x2_o], %[reg1]\n\t" \
      "bic %[reg3], %[x1_o], %[x0_o]\n\t" \
      "bic %[reg0], %[x3_o], %[x2_o]\n\t" \
      "eor %[x3_o], %[x3_o], %[reg2]\n\t" \
      "eor %[x3_o], %[x3_o], %[x2_o]\n\t" \
      "eor %[x4_o], %[x4_o], %[reg3]\n\t" \
      "eor %[x1_o], %[x1_o], %[reg0]\n\t" \
      "eor %[x3_e], %[x3_e], %[x2_e]\n\t" \
      "eor %[x1_e], %[x1_e], %[x0_e]\n\t" \
      "eor %[x1_o], %[x1_o], %[x0_o]\n\t" \
      "eor %[x0_e], %[x0_e], %[x4_e]\n\t" \
      "eor %[x0_o], %[x0_o], %[x4_o]\n\t" \
      "mvn %[x2_e], %[x2_e]\n\t" \
      "mvn %[x2_o], %[x2_o]\n\t" \
      "eor %[reg0], %[x0_e], %[x0_o], ror #4\n\t" \
      "eor %[reg1], %[x0_o], %[x0_e], ror #5\n\t" \
      "eor %[reg2], %[x1_e], %[x1_e], ror #11\n\t" \
      "eor %[reg3], %[x1_o], %[x1_o], ror #11\n\t" \
      "eor %[x0_e], %[x0_e], %[reg1], ror #9\n\t" \
      "eor %[x0_o], %[x0_o], %[reg0], ror #10\n\t" \
      "eor %[x1_e], %[x1_e], %[reg3], ror #19\n\t" \
      "eor %[x1_o], %[x1_o], %[reg2], ror #20\n\t" \
      "eor %[reg0], %[x2_e], %[x2_o], ror #2\n\t" \
      "eor %[reg1], %[x2_o], %[x2_e], ror #3\n\t" \
      "eor %[reg2], %[x3_e], %[x3_o], ror #3\n\t" \
      "eor %[reg3], %[x3_o], %[x3_e], ror #4\n\t" \
      "eor %[x2_e], %[x2_e], %[reg1]\n\t" \
      "eor %[x2_o], %[x2_o], %[reg0], ror #1\n\t" \
      "eor %[x3_e], %[x3_e], %[reg2], ror #5\n\t" \
      "eor %[x3_o], %[x3_o], %[reg3], ror #5\n\t" \
      "eor %[reg0], %[x4_e], %[x4_e], ror #17\n\t" \
      "eor %[reg1], %[x4_o], %[x4_o], ror #17\n\t" \
      "eor %[x4_e], %[x4_e], %[reg1], ror #3\n\t" \
      "eor %[x4_o], %[x4_o], %[reg0], ror #4\n\t" \
      : [x0_e] "+r" (x0.e), [x1_e] "+r" (x1.e), [x2_e] "+r" (x2.e), [x3_e] "+r" (x3.e), [x4_e] "+r" (x4.e), \
        [x0_o] "+r" (x0.o), [x1_o] "+r" (x1.o), [x2_o] "+r" (x2.o), [x3_o] "+r" (x3.o), [x4_o] "+r" (x4.o), \
        [reg0] "=r" (reg0), [reg1] "=r" (reg1), [reg2] "=r" (reg2), [reg3] "=r" (reg3)::); \
  } while (0)

#define P12()        \
  do {               \
    ROUND(0xc, 0xc); \
    ROUND(0x9, 0xc); \
    ROUND(0xc, 0x9); \
    ROUND(0x9, 0x9); \
    ROUND(0x6, 0xc); \
    ROUND(0x3, 0xc); \
    ROUND(0x6, 0x9); \
    ROUND(0x3, 0x9); \
    ROUND(0xc, 0x6); \
    ROUND(0x9, 0x6); \
    ROUND(0xc, 0x3); \
    ROUND(0x9, 0x3); \
  } while (0)

#define P8()         \
  do {               \
    ROUND(0x6, 0xc); \
    ROUND(0x3, 0xc); \
    ROUND(0x6, 0x9); \
    ROUND(0x3, 0x9); \
    ROUND(0xc, 0x6); \
    ROUND(0x9, 0x6); \
    ROUND(0xc, 0x3); \
    ROUND(0x9, 0x3); \
  } while (0)

#define P6()         \
  do {               \
    ROUND(0x6, 0x9); \
    ROUND(0x3, 0x9); \
    ROUND(0xc, 0x6); \
    ROUND(0x9, 0x6); \
    ROUND(0xc, 0x3); \
    ROUND(0x9, 0x3); \
  } while (0)

#endif  // PERMUTATIONS_H_

