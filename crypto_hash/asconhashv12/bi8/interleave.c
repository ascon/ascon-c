#include "interleave.h"

/* credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002 */
uint64_t interleave8(uint64_t x) {
  x = (x & 0xaa55aa55aa55aa55ull) | ((x & 0x00aa00aa00aa00aaull) << 7) |
      ((x >> 7) & 0x00aa00aa00aa00aaull);
  x = (x & 0xcccc3333cccc3333ull) | ((x & 0x0000cccc0000ccccull) << 14) |
      ((x >> 14) & 0x0000cccc0000ccccull);
  x = (x & 0xf0f0f0f00f0f0f0full) | ((x & 0x00000000f0f0f0f0ull) << 28) |
      ((x >> 28) & 0x00000000f0f0f0f0ull);
  return x;
}
