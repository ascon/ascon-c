#include "api.h"
#include "ascon.h"
#include "crypto_hash.h"
#include "permutations.h"
#include "printstate.h"

#if !ASCON_INLINE_MODE
#undef forceinline
#define forceinline
#endif

forceinline void ascon_hashinit(state_t* s) {
  /* initialize */
#if ASCON_HASH_OUTLEN == 32 && ASCON_HASH_ROUNDS == 12
  s->x0 = ASCON_HASH_IV0;
  s->x1 = ASCON_HASH_IV1;
  s->x2 = ASCON_HASH_IV2;
  s->x3 = ASCON_HASH_IV3;
  s->x4 = ASCON_HASH_IV4;
#elif ASCON_HASH_OUTLEN == 32 && ASCON_HASH_ROUNDS == 8
  s->x0 = ASCON_HASHA_IV0;
  s->x1 = ASCON_HASHA_IV1;
  s->x2 = ASCON_HASHA_IV2;
  s->x3 = ASCON_HASHA_IV3;
  s->x4 = ASCON_HASHA_IV4;
#elif ASCON_HASH_OUTLEN == 0 && ASCON_HASH_ROUNDS == 12
  s->x0 = ASCON_XOF_IV0;
  s->x1 = ASCON_XOF_IV1;
  s->x2 = ASCON_XOF_IV2;
  s->x3 = ASCON_XOF_IV3;
  s->x4 = ASCON_XOF_IV4;
#elif ASCON_HASH_OUTLEN == 0 && ASCON_HASH_ROUNDS == 8
  s->x0 = ASCON_XOFA_IV0;
  s->x1 = ASCON_XOFA_IV1;
  s->x2 = ASCON_XOFA_IV2;
  s->x3 = ASCON_XOFA_IV3;
  s->x4 = ASCON_XOFA_IV4;
#endif
  printstate("initialization", s);
}

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long inlen) {
  state_t s;
  ascon_hashinit(&s);
  ascon_update(&s, (void*)0, in, inlen, ASCON_HASH | ASCON_ABSORB);
  P(&s, 12);
  ascon_update(&s, out, (void*)0, CRYPTO_BYTES, ASCON_HASH | ASCON_SQUEEZE);
  return 0;
}
