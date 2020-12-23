#ifndef WORD_H_
#define WORD_H_

#include <stdint.h>

#include "endian.h"
#include "forceinline.h"
#include "interleave.h"

typedef union {
  uint64_t w;
  uint8_t b[8];
} word_t;

forceinline uint8_t ROR8(uint8_t a, int n) { return a >> n | a << (8 - n); }

forceinline word_t ROR(word_t a, int n) {
  word_t b;
  b.b[0] = ROR8(a.b[(n + 0) & 0x7], (n + 0) >> 3);
  b.b[1] = ROR8(a.b[(n + 1) & 0x7], (n + 1) >> 3);
  b.b[2] = ROR8(a.b[(n + 2) & 0x7], (n + 2) >> 3);
  b.b[3] = ROR8(a.b[(n + 3) & 0x7], (n + 3) >> 3);
  b.b[4] = ROR8(a.b[(n + 4) & 0x7], (n + 4) >> 3);
  b.b[5] = ROR8(a.b[(n + 5) & 0x7], (n + 5) >> 3);
  b.b[6] = ROR8(a.b[(n + 6) & 0x7], (n + 6) >> 3);
  b.b[7] = ROR8(a.b[(n + 7) & 0x7], (n + 7) >> 3);
  return b;
}

forceinline word_t WORD_T(uint64_t x) {
  word_t w;
  w.w = x;
  return w;
}

forceinline uint64_t UINT64_T(word_t w) {
  uint64_t x;
  x = w.w;
  return x;
}

forceinline word_t U64TOWORD(uint64_t x) { return WORD_T(interleave8(x)); }

forceinline uint64_t WORDTOU64(word_t w) { return interleave8(UINT64_T(w)); }

forceinline word_t NOT(word_t a) {
  a.w = ~a.w;
  return a;
}

forceinline word_t XOR(word_t a, word_t b) {
  a.w ^= b.w;
  return a;
}

forceinline word_t AND(word_t a, word_t b) {
  a.w &= b.w;
  return a;
}

forceinline word_t KEYROT(word_t lo2hi, word_t hi2lo) {
  word_t w;
  w.b[0] = lo2hi.b[0] << 4 | hi2lo.b[0] >> 4;
  w.b[1] = lo2hi.b[1] << 4 | hi2lo.b[1] >> 4;
  w.b[2] = lo2hi.b[2] << 4 | hi2lo.b[2] >> 4;
  w.b[3] = lo2hi.b[3] << 4 | hi2lo.b[3] >> 4;
  w.b[4] = lo2hi.b[4] << 4 | hi2lo.b[4] >> 4;
  w.b[5] = lo2hi.b[5] << 4 | hi2lo.b[5] >> 4;
  w.b[6] = lo2hi.b[6] << 4 | hi2lo.b[6] >> 4;
  w.b[7] = lo2hi.b[7] << 4 | hi2lo.b[7] >> 4;
  return w;
}

forceinline int NOTZERO(word_t a, word_t b) {
  uint64_t result = a.w | b.w;
  result |= result >> 32;
  result |= result >> 16;
  result |= result >> 8;
  return ((((int)(result & 0xff) - 1) >> 8) & 1) - 1;
}

forceinline word_t PAD(int i) { return (word_t){.b[7] = 0x80 >> i}; }

forceinline word_t CLEAR(word_t w, int n) {
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
  return AND(w, mask);
}

forceinline uint64_t MASK(int n) {
  /* undefined for n == 0 */
  return ~0ull >> (64 - 8 * n);
}

forceinline word_t LOAD(const uint8_t* bytes, int n) {
  uint64_t x = *(uint64_t*)bytes & MASK(n);
  return U64TOWORD(U64BIG(x));
}

forceinline void STORE(uint8_t* bytes, word_t w, int n) {
  uint64_t x = WORDTOU64(w);
  *(uint64_t*)bytes &= ~MASK(n);
  *(uint64_t*)bytes |= U64BIG(x);
}

forceinline word_t LOADBYTES(const uint8_t* bytes, int n) {
  uint64_t x = 0;
  for (int i = 0; i < n; ++i) ((uint8_t*)&x)[7 - i] = bytes[i];
  return U64TOWORD(x);
}

forceinline void STOREBYTES(uint8_t* bytes, word_t w, int n) {
  uint64_t x = WORDTOU64(w);
  for (int i = 0; i < n; ++i) bytes[i] = ((uint8_t*)&x)[7 - i];
}

#endif /* WORD_H_ */
