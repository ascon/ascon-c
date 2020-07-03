#include "permutations.h"

void P(state *p, u8 rounds) {
  state s = *p;
  u8 i, start = START_CONSTANT(rounds);
  for (i = start; i > 0x4a; i -= 0x0f) ROUND(i);
  *p = s;
}
