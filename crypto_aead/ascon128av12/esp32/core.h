#ifndef CORE_H_
#define CORE_H_

#include "api.h"
#include "bendian.h"
#include "permutations.h"

#define ASCON_AD 0
#define ASCON_ENC 1
#define ASCON_DEC 2

#define RATE (128 / 8)
#define PA_ROUNDS 12
#define PB_ROUNDS 8
#define PA_START_ROUND 0xf0
#define PB_START_ROUND 0xb4
#define IV                                                      \
  ((u64)(8 * (CRYPTO_KEYBYTES)) << 0 | (u64)(8 * (RATE)) << 8 | \
   (u64)(PA_ROUNDS) << 16 | (u64)(PB_ROUNDS) << 24)

void process_data(state* s, unsigned char* out, const unsigned char* in,
                  unsigned long long len, u8 mode);

void ascon_core(state* s, unsigned char* out, const unsigned char* in,
                unsigned long long tlen, const unsigned char* ad,
                unsigned long long adlen, const unsigned char* npub,
                const unsigned char* k, u8 mode);

#endif  // CORE_H_
