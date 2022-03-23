#include "permutations.h"

#if !ASCON_INLINE_PERM && ASCON_UNROLL_LOOPS

void P12(state_t* s) { P12ROUNDS(s); }

#endif

#if ((defined(ASCON_AEAD_RATE) && ASCON_AEAD_RATE == 16) ||     \
     (defined(ASCON_HASH_ROUNDS) && ASCON_HASH_ROUNDS == 8)) && \
    !ASCON_INLINE_PERM && ASCON_UNROLL_LOOPS

void P8(state_t* s) { P8ROUNDS(s); }

#endif

#if (defined(ASCON_AEAD_RATE) && ASCON_AEAD_RATE == 8) && \
    !ASCON_INLINE_PERM && ASCON_UNROLL_LOOPS

void P6(state_t* s) { P6ROUNDS(s); }

#endif

#if !ASCON_INLINE_PERM && !ASCON_UNROLL_LOOPS

void P(state_t* s, int nr) { PROUNDS(s, nr); }

#endif
