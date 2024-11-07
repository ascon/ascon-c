#ifndef PERMUTATIONS_H_
#define PERMUTATIONS_H_

#include <stdint.h>

#include "ascon.h"
#include "constants.h"
#include "printstate.h"
#include "round.h"

static inline void P12(ascon_state_t* s) {
  ROUND(s, RC0);
  ROUND(s, RC1);
  ROUND(s, RC2);
  ROUND(s, RC3);
  ROUND(s, RC4);
  ROUND(s, RC5);
  ROUND(s, RC6);
  ROUND(s, RC7);
  ROUND(s, RC8);
  ROUND(s, RC9);
  ROUND(s, RCa);
  ROUND(s, RCb);
}

static inline void P8(ascon_state_t* s) {
  ROUND(s, RC4);
  ROUND(s, RC5);
  ROUND(s, RC6);
  ROUND(s, RC7);
  ROUND(s, RC8);
  ROUND(s, RC9);
  ROUND(s, RCa);
  ROUND(s, RCb);
}

static inline void P6(ascon_state_t* s) {
  ROUND(s, RC6);
  ROUND(s, RC7);
  ROUND(s, RC8);
  ROUND(s, RC9);
  ROUND(s, RCa);
  ROUND(s, RCb);
}

#endif /* PERMUTATIONS_H_ */
