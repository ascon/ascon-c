#ifndef WORD_H_
#define WORD_H_

#include <stdint.h>
#include <string.h>

#include "bendian.h"
#include "forceinline.h"
#include "interleave.h"

typedef union {
  uint64_t x;
  uint32_t w[2];
  uint8_t b[8];
} word_t;

#define U64TOWORD(x) interleave8(U64BIG(x))
#define WORDTOU64(x) U64BIG(interleave8(x))

forceinline uint8_t ROR8(uint8_t a, int n) { return a >> n | a << (8 - n); }

forceinline uint64_t ROR(uint64_t x, int n) {
  word_t b, a = {.x = x};
  b.b[0] = ROR8(a.b[(n + 0) & 0x7], (n + 0) >> 3);
  b.b[1] = ROR8(a.b[(n + 1) & 0x7], (n + 1) >> 3);
  b.b[2] = ROR8(a.b[(n + 2) & 0x7], (n + 2) >> 3);
  b.b[3] = ROR8(a.b[(n + 3) & 0x7], (n + 3) >> 3);
  b.b[4] = ROR8(a.b[(n + 4) & 0x7], (n + 4) >> 3);
  b.b[5] = ROR8(a.b[(n + 5) & 0x7], (n + 5) >> 3);
  b.b[6] = ROR8(a.b[(n + 6) & 0x7], (n + 6) >> 3);
  b.b[7] = ROR8(a.b[(n + 7) & 0x7], (n + 7) >> 3);
  return b.x;
}

forceinline uint64_t KEYROT(uint64_t a, uint64_t b) {
  word_t w, lo2hi = {.x = a}, hi2lo = {.x = b};
  w.b[0] = lo2hi.b[0] << 4 | hi2lo.b[0] >> 4;
  w.b[1] = lo2hi.b[1] << 4 | hi2lo.b[1] >> 4;
  w.b[2] = lo2hi.b[2] << 4 | hi2lo.b[2] >> 4;
  w.b[3] = lo2hi.b[3] << 4 | hi2lo.b[3] >> 4;
  w.b[4] = lo2hi.b[4] << 4 | hi2lo.b[4] >> 4;
  w.b[5] = lo2hi.b[5] << 4 | hi2lo.b[5] >> 4;
  w.b[6] = lo2hi.b[6] << 4 | hi2lo.b[6] >> 4;
  w.b[7] = lo2hi.b[7] << 4 | hi2lo.b[7] >> 4;
  return w.x;
}

forceinline int NOTZERO(uint64_t a, uint64_t b) {
  uint64_t result = a | b;
  result |= result >> 32;
  result |= result >> 16;
  result |= result >> 8;
  return ((((int)(result & 0xff) - 1) >> 8) & 1) - 1;
}

forceinline uint64_t PAD(int i) { return (uint64_t)(0x80 >> i) << 56; }

forceinline uint64_t PRFS_MLEN(uint64_t len) {
  return ((len & 0x01) << 30) | /* 0000x */
         ((len & 0x02) << 37) | /* 000x0 */
         ((len & 0x04) << 44) | /* 00x00 */
         ((len & 0x08) << 51) | /* 0x000 */
         ((len & 0x10) << 58);  /* x0000 */
}

forceinline uint64_t CLEAR(uint64_t w, int n) {
  /* undefined for n == 0 */
  uint8_t m = 0xff >> n;
  word_t mask = {
      .b[0] = m,
      .b[1] = m,
      .b[2] = m,
      .b[3] = m,
      .b[4] = m,
      .b[5] = m,
      .b[6] = m,
      .b[7] = m,
  };
  return w & mask.x;
}

forceinline uint64_t MASK(int n) {
  /* undefined for n == 0 */
  return ~0ull >> (64 - 8 * n);
}

forceinline uint64_t LOAD(const uint8_t* bytes, int n) {
  uint64_t x = *(uint64_t*)bytes & MASK(n);
  return U64TOWORD(x);
}

forceinline void STORE(uint8_t* bytes, uint64_t w, int n) {
  *(uint64_t*)bytes &= ~MASK(n);
  *(uint64_t*)bytes |= WORDTOU64(w);
}

forceinline uint64_t LOADBYTES(const uint8_t* bytes, int n) {
  uint64_t x = 0;
  memcpy(&x, bytes, n);
  return U64TOWORD(x);
}

forceinline void STOREBYTES(uint8_t* bytes, uint64_t w, int n) {
  uint64_t x = WORDTOU64(w);
  memcpy(bytes, &x, n);
}

#endif /* WORD_H_ */
