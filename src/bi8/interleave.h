#ifndef INTERLEAVE_H_
#define INTERLEAVE_H_

#include <stdint.h>

#include "config.h"
#include "forceinline.h"

#if ASCON_EXTERN_BI

#define TOBI
#define FROMBI

#elif ASCON_INLINE_BI

#define TOBI interleave8
#define FROMBI interleave8

#else

uint64_t TOBI(uint64_t in);
uint64_t FROMBI(uint64_t in);

#endif

/* credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002 */
forceinline uint64_t interleave8(uint64_t x) {
  x = (x & 0xaa55aa55aa55aa55ULL) | ((x & 0x00aa00aa00aa00aaULL) << 7) |
      ((x >> 7) & 0x00aa00aa00aa00aaULL);
  x = (x & 0xcccc3333cccc3333ULL) | ((x & 0x0000cccc0000ccccULL) << 14) |
      ((x >> 14) & 0x0000cccc0000ccccULL);
  x = (x & 0xf0f0f0f00f0f0f0fULL) | ((x & 0x00000000f0f0f0f0ULL) << 28) |
      ((x >> 28) & 0x00000000f0f0f0f0ULL);
  return x;
}

#endif /* INTERLEAVE_H_ */
