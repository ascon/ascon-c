#include "permutations.h"

#include "lendian.h"
#include "printstate.h"

#define START_ROUND(x) (12 - (x))
#define LAST_ROUND 0x4b

#define SBOX(x0, x1, x2, x3, x4, r0, t0, t1, t2) \
  do {                                           \
    t1 = x0 ^ x4;                                \
    t2 = x3 ^ x4;                                \
    t0 = -1;                                     \
    x4 = x4 ^ t0;                                \
    t0 = x1 ^ x2;                                \
    x4 = x4 | x3;                                \
    x4 = x4 ^ t0;                                \
    x3 = x3 ^ x1;                                \
    x3 = x3 | t0;                                \
    x3 = x3 ^ t1;                                \
    x2 = x2 ^ t1;                                \
    x2 = x2 | x1;                                \
    x2 = x2 ^ t2;                                \
    x0 = x0 | t2;                                \
    x0 = x0 ^ t0;                                \
    t0 = -1;                                     \
    t1 = t1 ^ t0;                                \
    x1 = x1 & t1;                                \
    x1 = x1 ^ t2;                                \
    r0 = x0;                                     \
  } while (0)

#define SRC(o, h, l, amt)                 \
  do {                                    \
    o = (((uint64_t)h << 32) | l) >> amt; \
  } while (0)

#define LINEAR(dl, dh, sl, sh, sl0, sh0, r0, sl1, sh1, r1, t0) \
  do {                                                         \
    SRC(dl, sh0, sl0, r0);                                     \
    SRC(dh, sl0, sh0, r0);                                     \
    dl = dl ^ sl;                                              \
    dh = dh ^ sh;                                              \
    SRC(t0, sh1, sl1, r1);                                     \
    SRC(sh, sl1, sh1, r1);                                     \
    dl = dl ^ t0;                                              \
    dh = dh ^ sh;                                              \
  } while (0)

#if (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define lo h
#define hi l
#else
#define lo l
#define hi h
#endif

void P(ascon_state_t *p, uint8_t round_const) {
  uint32_t x0h = p->w[0].hi, x0l = p->w[0].lo;
  uint32_t x1h = p->w[1].hi, x1l = p->w[1].lo;
  uint32_t x2h = p->w[2].hi, x2l = p->w[2].lo;
  uint32_t x3h = p->w[3].hi, x3l = p->w[3].lo;
  uint32_t x4h = p->w[4].hi, x4l = p->w[4].lo;
  uint32_t t0l, t0h;
  uint32_t rnd = round_const;
  uint32_t tmp0;

  while (rnd >= LAST_ROUND) {
    x2l ^= rnd;

    SBOX(x0l, x1l, x2l, x3l, x4l, t0l, t0h, t0l, tmp0);
    SBOX(x0h, x1h, x2h, x3h, x4h, t0h, t0h, x0l, tmp0);

    LINEAR(x0l, x0h, x2l, x2h, x2l, x2h, 19, x2l, x2h, 28, tmp0);
    LINEAR(x2l, x2h, x4l, x4h, x4l, x4h, 1, x4l, x4h, 6, tmp0);
    LINEAR(x4l, x4h, x1l, x1h, x1l, x1h, 7, x1h, x1l, 9, tmp0);
    LINEAR(x1l, x1h, x3l, x3h, x3h, x3l, 29, x3h, x3l, 7, tmp0);
    LINEAR(x3l, x3h, t0l, t0h, t0l, t0h, 10, t0l, t0h, 17, tmp0);

    rnd -= 15;

#ifdef ASCON_PRINT_STATE
    p->w[0].hi = x0h;
    p->w[0].lo = x0l;
    p->w[1].hi = x1h;
    p->w[1].lo = x1l;
    p->w[2].hi = x2h;
    p->w[2].lo = x2l;
    p->w[3].hi = x3h;
    p->w[3].lo = x3l;
    p->w[4].hi = x4h;
    p->w[4].lo = x4l;
    printstate(" round output", p);
#endif
  }

  p->w[0].hi = x0h;
  p->w[0].lo = x0l;
  p->w[1].hi = x1h;
  p->w[1].lo = x1l;
  p->w[2].hi = x2h;
  p->w[2].lo = x2l;
  p->w[3].hi = x3h;
  p->w[3].lo = x3l;
  p->w[4].hi = x4h;
  p->w[4].lo = x4l;
}
