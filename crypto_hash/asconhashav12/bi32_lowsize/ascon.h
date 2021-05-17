#ifndef ASCON_H_
#define ASCON_H_

#include <stdint.h>

#include "word.h"

typedef struct {
  word_t x0, x1, x2, x3, x4;
} state_t;

#define ASCON_ABSORB 0x1
#define ASCON_SQUEEZE 0x2
#define ASCON_INSERT 0x4
#define ASCON_HASH 0x8
#define ASCON_ENCRYPT (ASCON_ABSORB | ASCON_SQUEEZE)
#define ASCON_DECRYPT (ASCON_ABSORB | ASCON_SQUEEZE | ASCON_INSERT)

void ascon_update(state_t* s, uint8_t* out, const uint8_t* in, uint64_t len,
                  uint8_t mode);

void ascon_aead(state_t* s, uint8_t* out, const uint8_t* in, uint64_t tlen,
                const uint8_t* ad, uint64_t adlen, const uint8_t* npub,
                const uint8_t* k, uint8_t mode);

#endif /* ASCON_H */
