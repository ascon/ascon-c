#include "interleave.h"

const uint32_t B[3] = {0x22222222, 0x0c0c0c0c, 0x00f000f0};

uint64_t TOBI(uint64_t in) { return deinterleave32(in); }
uint64_t FROMBI(uint64_t in) { return interleave32(in); }
