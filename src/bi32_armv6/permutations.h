#ifndef PERMUTATIONS_H_
#define PERMUTATIONS_H_

#include <stdint.h>

#include "api.h"
#include "ascon.h"
#include "config.h"
#include "constants.h"
#include "printstate.h"
#include "round.h"

#define LOADSTATE(s, a, b, c, d, e) \
  do {                              \
    a.x = s->x[0];                  \
    b.x = s->x[1];                  \
    c.x = s->x[2];                  \
    d.x = s->x[3];                  \
    e.x = s->x[4];                  \
  } while (0)

#define STORESTATE(s, a, b, c, d, e) \
  do {                               \
    s->x[0] = a.x;                   \
    s->x[1] = b.x;                   \
    s->x[2] = c.x;                   \
    s->x[3] = d.x;                   \
    s->x[4] = e.x;                   \
  } while (0)

forceinline void P12ROUNDS(ascon_state_t* s) {
  word_t x0, x1, x2, x3, x4;
  LOADSTATE(s, x0, x1, x2, x3, x4);
  ROUND5(x0, x1, x2, x3, x4, RC0);
  ROUND5(x2, x3, x4, x0, x1, RC1);
  ROUND5(x4, x0, x1, x2, x3, RC2);
  ROUND5(x1, x2, x3, x4, x0, RC3);
  ROUND5(x3, x4, x0, x1, x2, RC4);
  ROUND5(x0, x1, x2, x3, x4, RC5);
  ROUND5(x2, x3, x4, x0, x1, RC6);
  ROUND5(x4, x0, x1, x2, x3, RC7);
  ROUND5(x1, x2, x3, x4, x0, RC8);
  ROUND5(x3, x4, x0, x1, x2, RC9);
#if !ASCON_INLINE_PERM
  ROUND5(x0, x1, x2, x3, x4, RCa);
  ROUND5(x2, x3, x4, x0, x1, RCb);
  STORESTATE(s, x4, x0, x1, x2, x3);
#else
  STORESTATE(s, x0, x1, x2, x3, x4);
  ROUND(s, RCa);
  ROUND(s, RCb);
#endif
}

forceinline void P8ROUNDS(ascon_state_t* s) {
  word_t x0, x1, x2, x3, x4;
  LOADSTATE(s, x0, x1, x2, x3, x4);
  ROUND5(x0, x1, x2, x3, x4, RC4);
  ROUND5(x2, x3, x4, x0, x1, RC5);
  ROUND5(x4, x0, x1, x2, x3, RC6);
  ROUND5(x1, x2, x3, x4, x0, RC7);
  ROUND5(x3, x4, x0, x1, x2, RC8);
#if !ASCON_INLINE_PERM
  ROUND5(x0, x1, x2, x3, x4, RC9);
  ROUND5(x2, x3, x4, x0, x1, RCa);
  ROUND5(x4, x0, x1, x2, x3, RCb);
  STORESTATE(s, x1, x2, x3, x4, x0);
#else /* ASCON_INLINE_PERM */
  STORESTATE(s, x0, x1, x2, x3, x4);
  ROUND(s, RC9);
  ROUND(s, RCa);
  ROUND(s, RCb);
#endif
}

forceinline void P6ROUNDS(ascon_state_t* s) {
  word_t x0, x1, x2, x3, x4;
  LOADSTATE(s, x0, x1, x2, x3, x4);
  ROUND5(x0, x1, x2, x3, x4, RC6);
  ROUND5(x2, x3, x4, x0, x1, RC7);
  ROUND5(x4, x0, x1, x2, x3, RC8);
  ROUND5(x1, x2, x3, x4, x0, RC9);
  ROUND5(x3, x4, x0, x1, x2, RCa);
#if !ASCON_INLINE_PERM
  ROUND5(x0, x1, x2, x3, x4, RCb);
  STORESTATE(s, x2, x3, x4, x0, x1);
#else /* ASCON_INLINE_PERM */
  STORESTATE(s, x0, x1, x2, x3, x4);
  ROUND(s, RCb);
#endif
}

#if ASCON_INLINE_PERM && ASCON_UNROLL_LOOPS

forceinline void P(ascon_state_t* s, int nr) {
  if (nr == 12) P12ROUNDS(s);
  if (nr == 8) P8ROUNDS(s);
  if (nr == 6) P6ROUNDS(s);
}

#elif !ASCON_INLINE_PERM && ASCON_UNROLL_LOOPS

void P12(ascon_state_t* s);
void P8(ascon_state_t* s);
void P6(ascon_state_t* s);

forceinline void P(ascon_state_t* s, int nr) {
  if (nr == 12) P12(s);
  if (nr == 8) P8(s);
  if (nr == 6) P6(s);
}

#elif ASCON_INLINE_PERM && !ASCON_UNROLL_LOOPS

forceinline void P(ascon_state_t* s, int nr) { PROUNDS(s, nr); }

#else /* !ASCON_INLINE_PERM && !ASCON_UNROLL_LOOPS */

void P(ascon_state_t* s, int nr);

#endif

#endif /* PERMUTATIONS_H_ */
