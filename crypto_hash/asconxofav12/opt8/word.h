#ifndef WORD_H_
#define WORD_H_

#include <stdint.h>

#include "endian.h"
#include "forceinline.h"

typedef union {
  uint64_t w;
  uint8_t b[8];
} word_t;

#define U64TOWORD WORD_T
#define WORDTOU64 UINT64_T

#define XMUL(i, x)                               \
  do {                                           \
    tmp = (uint16_t)a.b[i] * (1 << (x));         \
    b.b[(byte_rol + (i)) & 0x7] ^= (uint8_t)tmp; \
    b.b[(byte_rol + (i) + 1) & 0x7] ^= tmp >> 8; \
  } while (0)

forceinline word_t ROR(word_t a, int n) {
  word_t b = {.w = 0ull};
  int bit_rol = (64 - n) & 0x7;
  int byte_rol = (64 - n) >> 3;
  uint16_t tmp;
  XMUL(0, bit_rol);
  XMUL(1, bit_rol);
  XMUL(2, bit_rol);
  XMUL(3, bit_rol);
  XMUL(4, bit_rol);
  XMUL(5, bit_rol);
  XMUL(6, bit_rol);
  XMUL(7, bit_rol);
  return b;
}

forceinline word_t WORD_T(uint64_t x) { return (word_t){.w = x}; }

forceinline uint64_t UINT64_T(word_t w) { return w.w; }

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
  return (word_t){.w = lo2hi.w << 32 | hi2lo.w >> 32};
}

forceinline int ISZERO(word_t a, word_t b) {
  uint64_t result = a.w | b.w;
  result |= result >> 32;
  result |= result >> 16;
  result |= result >> 8;
  return ((((int)(result & 0xff) - 1) >> 8) & 1) - 1;
}

forceinline word_t PAD(int i) { return WORD_T(0x80ull << (56 - 8 * i)); }

forceinline uint64_t MASK(int n) {
  /* undefined for n == 0 */
  return ~0ull >> (64 - 8 * n);
}

forceinline word_t CLEAR(word_t w, int n) {
  /* undefined for n == 0 */
  uint64_t mask = 0x00ffffffffffffffull >> (n * 8 - 8);
  return AND(w, WORD_T(mask));
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
