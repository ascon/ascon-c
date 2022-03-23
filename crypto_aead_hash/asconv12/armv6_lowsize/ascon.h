#ifndef ASCON_H_
#define ASCON_H_

#include <stdint.h>

#include "word.h"

typedef union {
  uint64_t x[5];
  uint32_t w[5][2];
  uint8_t b[5][8];
} state_t;

typedef struct {
#if (CRYPTO_KEYBYTES == 20)
  uint64_t k0;
#endif
  uint64_t k1;
  uint64_t k2;
} key_t;

#define ASCON_ABSORB 0x1
#define ASCON_SQUEEZE 0x2
#define ASCON_INSERT 0x4
#define ASCON_HASH 0x8
#define ASCON_ENCRYPT (ASCON_ABSORB | ASCON_SQUEEZE)
#define ASCON_DECRYPT (ASCON_ABSORB | ASCON_SQUEEZE | ASCON_INSERT)

void ascon_update(state_t* s, uint8_t* out, const uint8_t* in, uint64_t len,
                  uint8_t mode);

void ascon_aead(uint8_t* t, uint8_t* out, const uint8_t* in, uint64_t len,
                const uint8_t* ad, uint64_t adlen, const uint8_t* npub,
                const uint8_t* k, uint8_t mode);

#endif /* ASCON_H_ */
