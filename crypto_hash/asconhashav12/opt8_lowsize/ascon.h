#ifndef ASCON_H_
#define ASCON_H_

#include <stdint.h>

#include "api.h"

typedef union {
  uint64_t x[5];
  uint32_t w[5][2];
  uint8_t b[5][8];
} ascon_state_t;

#ifdef ASCON_AEAD_RATE

#define ASCON_KEYWORDS (CRYPTO_KEYBYTES + 7) / 8

typedef union {
  uint64_t x[ASCON_KEYWORDS];
  uint32_t w[ASCON_KEYWORDS][2];
  uint8_t b[ASCON_KEYWORDS][8];
} ascon_key_t;

#endif

#define ASCON_ABSORB 0x1
#define ASCON_SQUEEZE 0x2
#define ASCON_INSERT 0x4
#define ASCON_HASH 0x8
#define ASCON_ENCRYPT (ASCON_ABSORB | ASCON_SQUEEZE)
#define ASCON_DECRYPT (ASCON_ABSORB | ASCON_SQUEEZE | ASCON_INSERT)

void ascon_update(ascon_state_t* s, uint8_t* out, const uint8_t* in,
                  uint64_t len, uint8_t mode);

int ascon_aead(uint8_t* tag, uint8_t* out, const uint8_t* in, uint64_t inlen,
               const uint8_t* ad, uint64_t adlen, const uint8_t* npub,
               const uint8_t* k, uint8_t mode);

#endif /* ASCON_H_ */
