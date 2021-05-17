#include "api.h"
#include "ascon.h"
#include "permutations.h"
#include "printstate.h"

void ascon_update(state_t* s, uint8_t* out, const uint8_t* in, uint64_t len,
                  uint8_t mode) {
  int rate = (mode & ASCON_HASH) ? 8 : 16;
  const int nr = 8;
  word_t tmp0 = WORD_T(0), tmp1 = WORD_T(0);
  int n = 0, n0 = 0, n1 = 0;
  while (len) {
    /* determine block size */
    n0 = len < 8 ? len : 8;
    n1 = len < 8 ? 0 : (len < 16 ? len - 8 : 8);
    if (mode & ASCON_HASH) n1 = 0;
    n = n0 + n1;
    /* absorb data */
    if (mode & ASCON_ABSORB) {
      tmp0 = LOAD(in, n0);
      s->x0 = XOR(s->x0, tmp0);
      if (n1) tmp1 = LOAD(in + 8, n1);
      if (n1) s->x1 = XOR(s->x1, tmp1);
    }
    /* extract data */
    if (mode & ASCON_SQUEEZE) {
      STORE(out, s->x0, n0);
      if (n1) STORE(out + 8, s->x1, n1);
    }
    /* insert data */
    if (mode & ASCON_INSERT) {
      s->x0 = CLEAR(s->x0, n0);
      s->x0 = XOR(s->x0, tmp0);
      if (n1) s->x1 = CLEAR(s->x1, n1);
      if (n1) s->x1 = XOR(s->x1, tmp1);
    }
    /* compute permutation for full blocks */
    if (n == rate) P(s, nr);
    in += n;
    out += n;
    len -= n;
  }
  if (n % rate < 8)
    s->x0 = XOR(s->x0, PAD(n0 % 8));
  else
    s->x1 = XOR(s->x1, PAD(n1 % 8));
}
