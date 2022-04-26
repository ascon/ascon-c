#ifndef ASCON_H_
#define ASCON_H_

#include <stdint.h>

#include "word.h"

typedef struct {
  uint64_t x[5];
} ascon_state_t;

typedef struct {
#if (CRYPTO_KEYBYTES == 20)
  uint64_t k0;
#endif
  uint64_t k1;
  uint64_t k2;
} ascon_key_t;

#endif /* ASCON_H_ */
