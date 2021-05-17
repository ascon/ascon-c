#ifndef ASCON_H_
#define ASCON_H_

#include <immintrin.h>
#include <stdint.h>

#include "word.h"

typedef union {
  __m512i z;
  struct {
    word_t x0, x1, x2, x3, x4, x5, x6, x7;
  };
} state_t;

void ascon_aeadinit(state_t* s, const uint8_t* npub, const uint8_t* k);
void ascon_adata(state_t* s, const uint8_t* ad, uint64_t adlen);
void ascon_encrypt(state_t* s, uint8_t* c, const uint8_t* m, uint64_t mlen);
void ascon_decrypt(state_t* s, uint8_t* m, const uint8_t* c, uint64_t clen);
void ascon_final(state_t* s, const uint8_t* k);

#endif /* ASCON_H */
