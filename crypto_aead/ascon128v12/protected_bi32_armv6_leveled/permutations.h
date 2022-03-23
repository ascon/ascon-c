#ifndef PERMUTATIONS_H_
#define PERMUTATIONS_H_

#include "ascon.h"

void P1(state_t* s, int nr);
void P2(state_t* s, int nr);
void P3(state_t* s, int nr);
void P4(state_t* s, int nr);

forceinline void P(state_t* s, int nr, int ns) {
  if (ns == 1) P1(s, nr);
  if (ns == 2) P2(s, nr);
  if (ns == 3) P3(s, nr);
  if (ns == 4) P4(s, nr);
}

#endif /* PERMUTATIONS_H_ */
