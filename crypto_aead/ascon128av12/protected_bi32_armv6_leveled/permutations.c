#include "permutations.h"

#include "api.h"
#include "round.h"

#if NUM_SHARES_KEY == 1 || NUM_SHARES_AD == 1 || NUM_SHARES_M == 1
void P1(ascon_state_t* s, int nr) { PROUNDS(s, nr, 1); }
#endif

#if NUM_SHARES_KEY == 2 || NUM_SHARES_AD == 2 || NUM_SHARES_M == 2
void P2(ascon_state_t* s, int nr) { PROUNDS(s, nr, 2); }
#endif

#if NUM_SHARES_KEY == 3 || NUM_SHARES_AD == 3 || NUM_SHARES_M == 3
void P3(ascon_state_t* s, int nr) { PROUNDS(s, nr, 3); }
#endif

#if NUM_SHARES_KEY == 4 || NUM_SHARES_AD == 4 || NUM_SHARES_M == 4
void P4(ascon_state_t* s, int nr) { PROUNDS(s, nr, 4); }
#endif
