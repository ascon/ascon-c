#include "api.h"
#include "ascon.h"
#include "crypto_hash.h"
#include "permutations.h"
#include "printstate.h"

forceinline void ascon_inithash(ascon_state_t* s) {
  int i;
  /* initialize */
#ifdef ASCON_PRINT_STATE
#if ASCON_HASH_BYTES == 32 && ASCON_HASH_ROUNDS == 12
  s->x[0] = ASCON_HASH_IV;
#elif ASCON_HASH_BYTES == 32 && ASCON_HASH_ROUNDS == 8
  s->x[0] = ASCON_HASHA_IV;
#elif ASCON_HASH_BYTES == 0 && ASCON_HASH_ROUNDS == 12
  s->x[0] = ASCON_XOF_IV;
#elif ASCON_HASH_BYTES == 0 && ASCON_HASH_ROUNDS == 8
  s->x[0] = ASCON_XOFA_IV;
#endif
  for (i = 1; i < 5; ++i) s->x[i] = 0;
  printstate("initial value", s);
  P(s, 12);
#endif
#if ASCON_HASH_BYTES == 32 && ASCON_HASH_ROUNDS == 12
  const uint64_t iv[5] = {ASCON_HASH_IV0, ASCON_HASH_IV1, ASCON_HASH_IV2,
                          ASCON_HASH_IV3, ASCON_HASH_IV4};
#elif ASCON_HASH_BYTES == 32 && ASCON_HASH_ROUNDS == 8
  const uint64_t iv[5] = {ASCON_HASHA_IV0, ASCON_HASHA_IV1, ASCON_HASHA_IV2,
                          ASCON_HASHA_IV3, ASCON_HASHA_IV4};
#elif ASCON_HASH_BYTES == 0 && ASCON_HASH_ROUNDS == 12
  const uint64_t iv[5] = {ASCON_XOF_IV0, ASCON_XOF_IV1, ASCON_XOF_IV2,
                          ASCON_XOF_IV3, ASCON_XOF_IV4};
#elif ASCON_HASH_BYTES == 0 && ASCON_HASH_ROUNDS == 8
  const uint64_t iv[5] = {ASCON_XOFA_IV0, ASCON_XOFA_IV1, ASCON_XOFA_IV2,
                          ASCON_XOFA_IV3, ASCON_XOFA_IV4};
#endif
  for (i = 0; i < 5; ++i) s->x[i] = (iv[i]);
  printstate("initialization", s);
}

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long inlen) {
  ascon_state_t s;
  ascon_inithash(&s);
  ascon_update(ASCON_HASH | ASCON_ABSORB, &s, (void*)0, in, inlen);
  printstate("pad plaintext", &s);
  P(&s, 12);
  ascon_update(ASCON_HASH | ASCON_SQUEEZE, &s, out, (void*)0, CRYPTO_BYTES);
  return 0;
}
