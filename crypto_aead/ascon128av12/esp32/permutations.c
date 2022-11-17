#include "permutations.h"

#include "bendian.h"

u32_4 ascon_rev8(u32_4 in) {
  in.words[0].h = U32BIG(in.words[0].h);
  in.words[0].l = U32BIG(in.words[0].l);
  in.words[1].h = U32BIG(in.words[1].h);
  in.words[1].l = U32BIG(in.words[1].l);
  return in;
}

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

#define SRC(o, h, l, amt)            \
  do {                               \
    o = (((u64)h << 32) | l) >> amt; \
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

void P(state *p, u8 round_const, u8 rounds) {
  u32 x0h = p->x0.h, x0l = p->x0.l;
  u32 x1h = p->x1.h, x1l = p->x1.l;
  u32 x2h = p->x2.h, x2l = p->x2.l;
  u32 x3h = p->x3.h, x3l = p->x3.l;
  u32 x4h = p->x4.h, x4l = p->x4.l;
  u32 t0l, t0h;
  u32 rnd = round_const;
  u32 tmp0;

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
  }

  p->x0.h = x0h;
  p->x0.l = x0l;
  p->x1.h = x1h;
  p->x1.l = x1l;
  p->x2.h = x2h;
  p->x2.l = x2l;
  p->x3.h = x3h;
  p->x3.l = x3l;
  p->x4.h = x4h;
  p->x4.l = x4l;
}
