#ifndef ASCON_H_
#define ASCON_H_

#include <immintrin.h>
#include <stdint.h>

#include "word.h"

typedef union {
  __m512i z;
  uint64_t x[5];
} ascon_state_t;

typedef struct {
#if (CRYPTO_KEYBYTES == 20)
  uint64_t k0;
#endif
  uint64_t k1;
  uint64_t k2;
} ascon_key_t;

void ascon_initaead(ascon_state_t* s, const ascon_key_t* key,
                    const uint8_t* npub);
void ascon_adata(ascon_state_t* s, const uint8_t* ad, uint64_t adlen);
void ascon_encrypt(ascon_state_t* s, uint8_t* c, const uint8_t* m,
                   uint64_t mlen);
void ascon_decrypt(ascon_state_t* s, uint8_t* m, const uint8_t* c,
                   uint64_t clen);
void ascon_final(ascon_state_t* s, const ascon_key_t* k);

#endif /* ASCON_H_ */
