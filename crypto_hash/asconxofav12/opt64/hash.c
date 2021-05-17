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

forceinline void ascon_absorb(state_t* s, const uint8_t* in, uint64_t inlen) {
  /* absorb full plaintext blocks */
  while (inlen >= ASCON_HASH_RATE) {
    s->x0 = XOR(s->x0, LOAD(in, 8));
    P(s, ASCON_HASH_ROUNDS);
    in += ASCON_HASH_RATE;
    inlen -= ASCON_HASH_RATE;
  }
  /* absorb final plaintext block */
  if (inlen) s->x0 = XOR(s->x0, LOAD(in, inlen));
  s->x0 = XOR(s->x0, PAD(inlen));
  P(s, 12);
  printstate("absorb plaintext", s);
}

forceinline void ascon_squeeze(state_t* s, uint8_t* out, uint64_t outlen) {
  /* squeeze full output blocks */
  while (outlen > ASCON_HASH_RATE) {
    STORE(out, s->x0, 8);
    P(s, ASCON_HASH_ROUNDS);
    out += ASCON_HASH_RATE;
    outlen -= ASCON_HASH_RATE;
  }
  /* squeeze final output block */
  STORE(out, s->x0, outlen);
  printstate("squeeze output", s);
}

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long inlen) {
  state_t s;
  ascon_hashinit(&s);
  ascon_absorb(&s, in, inlen);
  ascon_squeeze(&s, out, CRYPTO_BYTES);
  return 0;
}
