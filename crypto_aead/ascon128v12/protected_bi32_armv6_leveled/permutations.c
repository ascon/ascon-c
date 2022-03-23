#include "permutations.h"

#include "api.h"
#include "round.h"

#if NUM_SHARES_KEY == 1 || NUM_SHARES_AD == 1 || NUM_SHARES_MC == 1
void P1(state_t* s, int nr) { PROUNDS(s, nr, 1); }
#endif

#if NUM_SHARES_KEY == 2 || NUM_SHARES_AD == 2 || NUM_SHARES_MC == 2
void P2(state_t* s, int nr) { PROUNDS(s, nr, 2); }
#endif

#if NUM_SHARES_KEY == 3 || NUM_SHARES_AD == 3 || NUM_SHARES_MC == 3
void P3(state_t* s, int nr) { PROUNDS(s, nr, 3); }
#endif

#if NUM_SHARES_KEY == 4 || NUM_SHARES_AD == 4 || NUM_SHARES_MC == 4
void P4(state_t* s, int nr) { PROUNDS(s, nr, 4); }
#endif
