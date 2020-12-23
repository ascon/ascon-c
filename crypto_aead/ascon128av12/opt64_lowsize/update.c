#include "api.h"
#include "ascon.h"
#include "permutations.h"
#include "printstate.h"

void ascon_update(state_t* s, uint8_t* out, const uint8_t* in, uint64_t len,
                  uint8_t mode) {
  const int nr = (ASCON_RATE == 8) ? 6 : 8;
  word_t tmp0, tmp1;
  /* full blocks */
  while (len >= ASCON_RATE) {
    tmp0 = LOAD(in, 8);
    tmp1 = LOAD(in + 8, 8);
    s->x0 = XOR(s->x0, tmp0);
    s->x1 = XOR(s->x1, tmp1);
    if (mode & ASCON_SQUEEZE) {
      STORE(out, s->x0, 8);
      STORE(out + 8, s->x1, 8);
    }
    if (mode & ASCON_INSERT) {
      s->x0 = tmp0;
      s->x1 = tmp1;
    }
    P(s, nr);
    in += ASCON_RATE;
    out += ASCON_RATE;
    len -= ASCON_RATE;
  }
  /* final block */
  if (len) {
    tmp1 = WORD_T(0);
    if (len >= 8) tmp0 = LOAD(in, 8);
    if (len > 8)
      tmp1 = LOAD(in + 8, len - 8);
    else
      tmp0 = LOAD(in, len);
    s->x0 = XOR(s->x0, tmp0);
    s->x1 = XOR(s->x1, tmp1);
    if (mode & ASCON_SQUEEZE) {
      if (len >= 8) STORE(out, s->x0, 8);
      if (len > 8)
        STORE(out + 8, s->x1, len - 8);
      else
        STORE(out, s->x0, len);
    }
    if (mode & ASCON_INSERT) {
      if (len >= 8) s->x0 = tmp0;
      if (len > 8) {
        s->x1 = CLEAR(s->x1, len - 8);
        s->x1 = XOR(s->x1, tmp1);
      } else {
        s->x0 = CLEAR(s->x0, len);
        s->x0 = XOR(s->x0, tmp0);
      }
    }
  }
  if (len < 8)
    s->x0 = XOR(s->x0, PAD(len % 8));
  else
    s->x1 = XOR(s->x1, PAD(len % 8));
}
