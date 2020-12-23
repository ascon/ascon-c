#include "api.h"
#include "ascon.h"
#include "permutations.h"
#include "printstate.h"

void ascon_update(state_t* s, uint8_t* out, const uint8_t* in, uint64_t len,
                  uint8_t mode) {
  const int nr = (ASCON_RATE == 8) ? 6 : 8;
  word_t tmp0;
  int n = 0;
  while (len) {
    /* determine block size */
    n = len < ASCON_RATE ? len : ASCON_RATE;
    /* absorb data */
    tmp0 = LOAD(in, n);
    s->x0 = XOR(s->x0, tmp0);
    /* extract data */
    if (mode & ASCON_SQUEEZE) STORE(out, s->x0, n);
    /* insert data */
    if (mode & ASCON_INSERT) {
      s->x0 = CLEAR(s->x0, n);
      s->x0 = XOR(s->x0, tmp0);
    }
    /* compute permutation for full blocks */
    if (n == ASCON_RATE) P(s, nr);
    in += n;
    out += n;
    len -= n;
  }
  s->x0 = XOR(s->x0, PAD(n % 8));
}
