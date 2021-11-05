#include "endian.h"
#include "permutations.h"

static const u8 constants[][2] = {
    {0xc, 0xc}, {0x9, 0xc}, {0xc, 0x9}, {0x9, 0x9}, {0x6, 0xc}, {0x3, 0xc},
    {0x6, 0x9}, {0x3, 0x9}, {0xc, 0x6}, {0x9, 0x6}, {0xc, 0x3}, {0x9, 0x3}};

// Credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002
u32_2 to_bit_interleaving_big(u64 in) {
  u32_2 out;
  to_bit_interleaving_big_immediate(out, in);
  return out;
}

// Credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002
u64 from_bit_interleaving_big(u32_2 in) {
  u64 out;
  from_bit_interleaving_big_immediate(out, in);
  return out;
}

void P(state *p, u8 rounds) {
  state s = *p;
  u32_2 t0;
  u32 i, start = START_ROUND(rounds);
  for (i = start; i < 12; i++) ROUND(constants[i][0], constants[i][1]);
  *p = s;
}
