#include "permutations.h"

static const u8 constants[][2] = {
    {0xc, 0xc}, {0x9, 0xc}, {0xc, 0x9}, {0x9, 0x9}, {0x6, 0xc}, {0x3, 0xc},
    {0x6, 0x9}, {0x3, 0x9}, {0xc, 0x6}, {0x9, 0x6}, {0xc, 0x3}, {0x9, 0x3}};

/* Credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002 */
u32_2 to_bit_interleaving(u64 in) {
  u32 hi = (in) >> 32;
  u32 lo = (u32)(in);
  u32 r0, r1;
  u32_2 out;
  r0 = (lo ^ (lo >> 1)) & 0x22222222, lo ^= r0 ^ (r0 << 1);
  r0 = (lo ^ (lo >> 2)) & 0x0C0C0C0C, lo ^= r0 ^ (r0 << 2);
  r0 = (lo ^ (lo >> 4)) & 0x00F000F0, lo ^= r0 ^ (r0 << 4);
  r0 = (lo ^ (lo >> 8)) & 0x0000FF00, lo ^= r0 ^ (r0 << 8);
  r1 = (hi ^ (hi >> 1)) & 0x22222222, hi ^= r1 ^ (r1 << 1);
  r1 = (hi ^ (hi >> 2)) & 0x0C0C0C0C, hi ^= r1 ^ (r1 << 2);
  r1 = (hi ^ (hi >> 4)) & 0x00F000F0, hi ^= r1 ^ (r1 << 4);
  r1 = (hi ^ (hi >> 8)) & 0x0000FF00, hi ^= r1 ^ (r1 << 8);
  out.e = (lo & 0x0000FFFF) | (hi << 16);
  out.o = (lo >> 16) | (hi & 0xFFFF0000);
  return out;
}

/* Credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002 */
u64 from_bit_interleaving(u32_2 in) {
  u32 lo = (in.e & 0x0000FFFF) | (in.o << 16);
  u32 hi = (in.e >> 16) | (in.o & 0xFFFF0000);
  u32 r0, r1;
  u64 out;
  r0 = (lo ^ (lo >> 8)) & 0x0000FF00, lo ^= r0 ^ (r0 << 8);
  r0 = (lo ^ (lo >> 4)) & 0x00F000F0, lo ^= r0 ^ (r0 << 4);
  r0 = (lo ^ (lo >> 2)) & 0x0C0C0C0C, lo ^= r0 ^ (r0 << 2);
  r0 = (lo ^ (lo >> 1)) & 0x22222222, lo ^= r0 ^ (r0 << 1);
  r1 = (hi ^ (hi >> 8)) & 0x0000FF00, hi ^= r1 ^ (r1 << 8);
  r1 = (hi ^ (hi >> 4)) & 0x00F000F0, hi ^= r1 ^ (r1 << 4);
  r1 = (hi ^ (hi >> 2)) & 0x0C0C0C0C, hi ^= r1 ^ (r1 << 2);
  r1 = (hi ^ (hi >> 1)) & 0x22222222, hi ^= r1 ^ (r1 << 1);
  out = (u64)hi << 32 | lo;
  return out;
}

void P(state *p, u8 rounds) {
  state s = *p;
  u32_2 t0, t1, t2, t3, t4;
  u32 i, start = START_ROUND(rounds);
  for (i = start; i < 12; i++) ROUND(constants[i][0], constants[i][1]);
  *p = s;
}
