#include "permutations.h"

#if !ASCON_UNROLL_LOOPS

const uint64_t constants[12] = {
    0x0101010100000000ull, 0x0101010000000001ull, 0x0101000100000100ull,
    0x0101000000000101ull, 0x0100010100010000ull, 0x0100010000010001ull,
    0x0100000100010100ull, 0x0100000000010101ull, 0x0001010101000000ull,
    0x0001010001000001ull, 0x0001000101000100ull, 0x0001000001000101ull};

#endif

#if !ASCON_INLINE_PERM && ASCON_UNROLL_LOOPS

void P12(state_t* s) { P12ROUNDS(s); }
void P8(state_t* s) { P8ROUNDS(s); }
void P6(state_t* s) { P6ROUNDS(s); }

#endif

#if !ASCON_INLINE_PERM && !ASCON_UNROLL_LOOPS

void P(state_t* s, int nr) { PROUNDS(s, nr); }

#endif
