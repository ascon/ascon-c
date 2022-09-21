#include "interleave.h"

#if !ASCON_INLINE_BI

uint64_t TOBI(uint64_t in) { return interleave8(in); }

uint64_t FROMBI(uint64_t in) { return interleave8(in); }

#endif