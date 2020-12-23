#ifndef INTERLEAVE_H_
#define INTERLEAVE_H_

#include <stdint.h>

#include "forceinline.h"

uint64_t deinterleave32(uint64_t in);
uint64_t interleave32(uint64_t in);

#endif /* INTERLEAVE_H_ */
