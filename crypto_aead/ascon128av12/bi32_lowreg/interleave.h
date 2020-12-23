#ifndef INTERLEAVE_H_
#define INTERLEAVE_H_

#include <stdint.h>

#include "forceinline.h"

forceinline uint32_t deinterleave_uint32(uint32_t x) {
  uint32_t t;
  t = (x ^ (x >> 1)) & 0x22222222, x ^= t ^ (t << 1);
  t = (x ^ (x >> 2)) & 0x0C0C0C0C, x ^= t ^ (t << 2);
  t = (x ^ (x >> 4)) & 0x00F000F0, x ^= t ^ (t << 4);
  t = (x ^ (x >> 8)) & 0x0000FF00, x ^= t ^ (t << 8);
  return x;
}

forceinline uint32_t interleave_uint32(uint32_t x) {
  uint32_t t;
  t = (x ^ (x >> 8)) & 0x0000FF00, x ^= t ^ (t << 8);
  t = (x ^ (x >> 4)) & 0x00F000F0, x ^= t ^ (t << 4);
  t = (x ^ (x >> 2)) & 0x0C0C0C0C, x ^= t ^ (t << 2);
  t = (x ^ (x >> 1)) & 0x22222222, x ^= t ^ (t << 1);
  return x;
}

/* credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002 */
forceinline uint64_t deinterleave32(uint64_t in) {
  uint32_t hi = in >> 32;
  uint32_t lo = in;
  uint32_t r0, r1;
  lo = deinterleave_uint32(lo);
  hi = deinterleave_uint32(hi);
  r0 = (lo & 0x0000FFFF) | (hi << 16);
  r1 = (lo >> 16) | (hi & 0xFFFF0000);
  return (uint64_t)r1 << 32 | r0;
}

/* credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002 */
forceinline uint64_t interleave32(uint64_t in) {
  uint32_t r0 = in;
  uint32_t r1 = in >> 32;
  uint32_t lo = (r0 & 0x0000FFFF) | (r1 << 16);
  uint32_t hi = (r0 >> 16) | (r1 & 0xFFFF0000);
  lo = interleave_uint32(lo);
  hi = interleave_uint32(hi);
  return (uint64_t)hi << 32 | lo;
}

#endif /* INTERLEAVE_H_ */
