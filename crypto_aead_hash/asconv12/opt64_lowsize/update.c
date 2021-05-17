#include "api.h"
#include "ascon.h"
#include "permutations.h"
#include "printstate.h"

void ascon_update(state_t* s, uint8_t* out, const uint8_t* in, uint64_t len,
                  uint8_t mode) {
  const int rate = 8;
  int nr = (mode & ASCON_HASH) ? 12 : 6;
  word_t tmp0 = WORD_T(0);
  int n = 0;
  while (len) {
    /* determine block size */
    n = len < rate ? len : rate;
    /* absorb data */
    if (mode & ASCON_ABSORB) {
      tmp0 = LOAD(in, n);
      s->x0 = XOR(s->x0, tmp0);
    }
    /* extract data */
    if (mode & ASCON_SQUEEZE) STORE(out, s->x0, n);
    /* insert data */
    if (mode & ASCON_INSERT) {
      s->x0 = CLEAR(s->x0, n);
      s->x0 = XOR(s->x0, tmp0);
    }
    /* compute permutation for full blocks */
    if (n == rate) P(s, nr);
    in += n;
    out += n;
    len -= n;
  }
  s->x0 = XOR(s->x0, PAD(n % 8));
}
