#ifndef PERMUTATIONS_H_
#define PERMUTATIONS_H_

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef struct {
  u32 l;
  u32 h;
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
u32_2 ascon_rev8_half(u32_2 in);

void P(state *p, u8 round_const);

#endif  // PERMUTATIONS_H_
