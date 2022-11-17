#ifndef WORD_H_
#define WORD_H_

#include <stdint.h>
#include <string.h>

#include "bendian.h"
#include "config.h"
#include "forceinline.h"
#include "interleave.h"

#if ASCON_EXTERN_BI

#define U64TOWORD(x) U64BIG(x)
#define WORDTOU64(x) U64BIG(x)

#else

#define U64TOWORD(x) TOBI(x)
#define WORDTOU64(x) FROMBI(x)

#endif

typedef union {
  uint64_t x;
  uint32_t w[2];
  uint8_t b[8];
} word_t;

forceinline uint32_t ROR32(uint32_t x, int n) {
  return x >> n | x << (-n & 31);
}

forceinline uint64_t ROR(uint64_t x, int n) {
  word_t b, a = {.x = x};
  b.w[0] = (n % 2) ? ROR32(a.w[1], (n - 1) / 2) : ROR32(a.w[0], n / 2);
  b.w[1] = (n % 2) ? ROR32(a.w[0], (n + 1) / 2) : ROR32(a.w[1], n / 2);
  return b.x;
}

forceinline uint64_t KEYROT(uint64_t a, uint64_t b) {
  word_t w, lo2hi = {.x = a}, hi2lo = {.x = b};
  w.w[0] = lo2hi.w[0] << 16 | hi2lo.w[0] >> 16;
  w.w[1] = lo2hi.w[1] << 16 | hi2lo.w[1] >> 16;
  return w.x;
}

forceinline int NOTZERO(uint64_t a, uint64_t b) {
  uint64_t result = a | b;
  result |= result >> 32;
  result |= result >> 16;
  result |= result >> 8;
  return ((((int)(result & 0xff) - 1) >> 8) & 1) - 1;
}

#if ASCON_EXTERN_BI

forceinline uint64_t PAD(int i) { return 0x80ull << (56 - 8 * i); }

forceinline uint64_t PRFS_MLEN(uint64_t len) { return len << 51; }

forceinline uint64_t CLEAR(uint64_t w, int n) {
  /* undefined for n == 0 */
  uint64_t mask = ~0ull >> (8 * n);
  return w & mask;
}

#else

forceinline uint64_t PAD(int i) {
  return ((uint64_t)((uint32_t)0x08 << (28 - 4 * i)) << 32);
}

forceinline uint64_t PRFS_MLEN(uint64_t len) {
  return ((len & 0x01) << 57) | /* 0000x */
         ((len & 0x02) << 25) | /* 000x0 */
         ((len & 0x04) << 56) | /* 00x00 */
         ((len & 0x08) << 24) | /* 0x000 */
         ((len & 0x10) << 55);  /* x0000 */
}

forceinline uint64_t CLEAR(uint64_t w, int n) {
  /* undefined for n == 0 */
  uint32_t mask = 0xffffffffull >> (4 * n);
  return w & ((uint64_t)mask << 32 | mask);
}

#endif

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
