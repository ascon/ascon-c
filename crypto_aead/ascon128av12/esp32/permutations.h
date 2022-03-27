#ifndef PERMUTATIONS_H_
#define PERMUTATIONS_H_

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef struct {
  u32 h;
  u32 l;
} u32_2;

typedef struct {
  u32_2 words[2];
} u32_4;

typedef struct {
  u32_2 x0;
  u32_2 x1;
  u32_2 x2;
  u32_2 x3;
  u32_2 x4;
} state;

#define START_ROUND(x) (12 - (x))
#define LAST_ROUND 0x4b

u32_4 ascon_rev8(u32_4 in);

#define to_big_immediate(out, in) \
  do {                            \
    u64 big_in = U64BIG(in);      \
    u32 hi = (big_in) >> 32;      \
    u32 lo = (u32)(big_in);       \
    out.h = hi;                   \
    out.l = lo;                   \
  } while (0)

#define from_big_immediate(out, in) \
  do {                              \
    u32 hi = in.h;                  \
    u32 lo = in.l;                  \
    out = (u64)hi << 32 | lo;       \
    out = U64BIG(out);              \
  } while (0)

void P(state *p, u8 round_const, u8 rounds);

#endif  // PERMUTATIONS_H_
