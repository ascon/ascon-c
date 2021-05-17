#ifndef WORD_H_
#define WORD_H_

#include <stdint.h>

#include "endian.h"
#include "forceinline.h"
#include "interleave.h"

typedef struct {
  uint32_t e;
  uint32_t o;
} word_t;

forceinline uint32_t ROR32(uint32_t x, int n) {
  return (n == 0) ? x : x >> n | x << (32 - n);
}

forceinline word_t ROR(word_t x, int n) {
  word_t r;
  r.e = (n % 2) ? ROR32(x.o, (n - 1) / 2) : ROR32(x.e, n / 2);
  r.o = (n % 2) ? ROR32(x.e, (n + 1) / 2) : ROR32(x.o, n / 2);
  return r;
}

forceinline word_t WORD_T(uint64_t x) { return (word_t){.o = x >> 32, .e = x}; }

forceinline uint64_t UINT64_T(word_t x) { return (uint64_t)x.o << 32 | x.e; }

forceinline word_t U64TOWORD(uint64_t x) { return WORD_T(deinterleave32(x)); }

forceinline uint64_t WORDTOU64(word_t w) { return interleave32(UINT64_T(w)); }

forceinline word_t NOT(word_t a) {
  a.e = ~a.e;
  a.o = ~a.o;
  return a;
}

forceinline word_t XOR(word_t a, word_t b) {
  a.e ^= b.e;
  a.o ^= b.o;
  return a;
}

forceinline word_t AND(word_t a, word_t b) {
  a.e &= b.e;
  a.o &= b.o;
  return a;
}

forceinline word_t KEYROT(word_t lo2hi, word_t hi2lo) {
  word_t r;
  r.e = lo2hi.e << 16 | hi2lo.e >> 16;
  r.o = lo2hi.o << 16 | hi2lo.o >> 16;
  return r;
}

forceinline int NOTZERO(word_t a, word_t b) {
  uint32_t result = a.e | a.o | b.e | b.o;
  result |= result >> 16;
  result |= result >> 8;
  return ((((int)(result & 0xff) - 1) >> 8) & 1) - 1;
}

forceinline word_t PAD(int i) {
  return WORD_T((uint64_t)(0x8ul << (28 - 4 * i)) << 32);
}

forceinline word_t CLEAR(word_t w, int n) {
  /* undefined for n == 0 */
  uint32_t mask = 0x0fffffff >> (n * 4 - 4);
  w.e &= mask;
  w.o &= mask;
  return w;
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
