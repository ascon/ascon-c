#include "api.h"
#include "ascon.h"
#include "crypto_hash.h"
#include "permutations.h"
#include "printstate.h"

#if ASCON_HASH_BYTES == 32 && ASCON_HASH_ROUNDS == 12
#define IV(i) ASCON_HASH_IV##i
#elif ASCON_HASH_BYTES == 32 && ASCON_HASH_ROUNDS == 8
#define IV(i) ASCON_HASHA_IV##i
#elif ASCON_HASH_BYTES == 0 && ASCON_HASH_ROUNDS == 12
#define IV(i) ASCON_XOF_IV##i
#elif ASCON_HASH_BYTES == 0 && ASCON_HASH_ROUNDS == 8
#define IV(i) ASCON_XOFA_IV##i
#endif

forceinline void ascon_inithash(ascon_state_t* s) {
  /* initialize */
#ifdef ASCON_PRINT_STATE
  *s = (ascon_state_t){{IV(), 0, 0, 0, 0}};
  printstate("initial value", s);
  P(s, 12);
#else
  *s = (ascon_state_t){{IV(0), IV(1), IV(2), IV(3), IV(4)}};
#endif
  printstate("initialization", s);
}

int ascon_xof(uint8_t* out, uint64_t outlen, const uint8_t* in,
              uint64_t inlen) {
  ascon_state_t s;
  ascon_inithash(&s);
  ascon_update(&s, (void*)0, in, inlen, ASCON_HASH | ASCON_ABSORB);
  printstate("pad plaintext", &s);
  P(&s, 12);
  ascon_update(&s, out, (void*)0, outlen, ASCON_HASH | ASCON_SQUEEZE);
  return 0;
}

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long inlen) {
  return ascon_xof(out, CRYPTO_BYTES, in, inlen);
}
