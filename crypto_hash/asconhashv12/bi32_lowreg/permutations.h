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
#define ROTR32(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

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
    /* round constant */ \
    x2.e ^= C_e;              x2.o ^= C_o; \
    /* s-box layer */ \
    x0.e ^= x4.e;             x0.o ^= x4.o; \
    x4.e ^= x3.e;             x4.o ^= x3.o; \
    x2.e ^= x1.e;             x2.o ^= x1.o; \
    t0.e = x0.e & (~x4.e);    t0.o = x0.o & (~x4.o); \
    x0.e ^= x2.e & (~x1.e);   x0.o ^= x2.o & (~x1.o); \
    x2.e ^= x4.e & (~x3.e);   x2.o ^= x4.o & (~x3.o); \
    x4.e ^= x1.e & (~x0.e);   x4.o ^= x1.o & (~x0.o); \
    x1.e ^= x3.e & (~x2.e);   x1.o ^= x3.o & (~x2.o); \
    x3.e ^= t0.e;             x3.o ^= t0.o; \
    x1.e ^= x0.e;             x1.o ^= x0.o; \
    x3.e ^= x2.e;             x3.o ^= x2.o; \
    x0.e ^= x4.e;             x0.o ^= x4.o; \
    /* linear layer */ \
    t0.e  = x0.e ^ ROTR32(x0.o, 4); \
    t0.o  = x0.o ^ ROTR32(x0.e, 5); \
    x0.e ^= ROTR32(t0.o, 9); \
    x0.o ^= ROTR32(t0.e, 10); \
    t0.e  = x1.e ^ ROTR32(x1.e, 11); \
    t0.o  = x1.o ^ ROTR32(x1.o, 11); \
    x1.e ^= ROTR32(t0.o, 19); \
    x1.o ^= ROTR32(t0.e, 20); \
    t0.e  = x2.e ^ ROTR32(x2.o, 2); \
    t0.o  = x2.o ^ ROTR32(x2.e, 3); \
    x2.e ^= t0.o; \
    x2.o ^= ROTR32(t0.e, 1); \
    t0.e  = x3.e ^ ROTR32(x3.o, 3); \
    t0.o  = x3.o ^ ROTR32(x3.e, 4); \
    x3.e ^= ROTR32(t0.e, 5); \
    x3.o ^= ROTR32(t0.o, 5); \
    t0.e  = x4.e ^ ROTR32(x4.e, 17); \
    t0.o  = x4.o ^ ROTR32(x4.o, 17); \
    x4.e ^= ROTR32(t0.o, 3); \
    x4.o ^= ROTR32(t0.e, 4); \
    x2.e = ~x2.e;             x2.o = ~x2.o; \
  } while(0)

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

