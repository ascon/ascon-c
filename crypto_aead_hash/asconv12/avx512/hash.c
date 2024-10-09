#include "api.h"
#include "ascon.h"
#include "crypto_hash.h"
#include "permutations.h"
#include "printstate.h"

#define AVX512_SHUFFLE_U64BIG                                  \
  _mm512_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, /* word 7 */ \
                  -1, -1, -1, -1, -1, -1, -1, -1, /* word 6 */ \
                  -1, -1, -1, -1, -1, -1, -1, -1, /* word 5 */ \
                  -1, -1, -1, -1, -1, -1, -1, -1, /* word 4 */ \
                  -1, -1, -1, -1, -1, -1, -1, -1, /* word 3 */ \
                  -1, -1, -1, -1, -1, -1, -1, -1, /* word 2 */ \
                  8, 9, 10, 11, 12, 13, 14, 15,   /* word 1 */ \
                  0, 1, 2, 3, 4, 5, 6, 7)         /* word 0 */

#if !ASCON_INLINE_MODE
#undef forceinline
#define forceinline
#endif

#ifdef ASCON_HASH_BYTES

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
#else
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
  for (i = 0; i < 5; ++i) s->x[i] = iv[i];
#endif
  printstate("initialization", s);
}

forceinline void ascon_absorb(ascon_state_t* s, const uint8_t* in,
                              uint64_t inlen) {
  const __m512i u64big = AVX512_SHUFFLE_U64BIG;
  /* absorb full plaintext blocks */
  while (inlen >= ASCON_HASH_RATE) {
    ascon_state_t t;
    t.z = _mm512_maskz_loadu_epi8(0xff, in);
    t.z = _mm512_maskz_shuffle_epi8(0xff, t.z, u64big);
    s->z = _mm512_xor_epi64(s->z, t.z);
    printstate("absorb plaintext", s);
    P(s, ASCON_HASH_ROUNDS);
    in += ASCON_HASH_RATE;
    inlen -= ASCON_HASH_RATE;
  }
  /* absorb final plaintext block */
  s->x[0] ^= LOADBYTES(in, inlen);
  s->x[0] ^= PAD(inlen);
  printstate("pad plaintext", s);
}

forceinline void ascon_squeeze(ascon_state_t* s, uint8_t* out,
                               uint64_t outlen) {
  const __m512i u64big = AVX512_SHUFFLE_U64BIG;
  /* squeeze full output blocks */
  P(s, 12);
  while (outlen > ASCON_HASH_RATE) {
    ascon_state_t t;
    t.z = _mm512_maskz_shuffle_epi8(0xff, s->z, u64big);
    _mm512_mask_storeu_epi8(out, 0xff, t.z);
    printstate("squeeze output", s);
    P(s, ASCON_HASH_ROUNDS);
    out += ASCON_HASH_RATE;
    outlen -= ASCON_HASH_RATE;
  }
  /* squeeze final output block */
  STOREBYTES(out, s->x[0], outlen);
  printstate("squeeze output", s);
}

int ascon_xof(uint8_t* out, uint64_t outlen, const uint8_t* in,
              uint64_t inlen) {
  ascon_state_t s;
  ascon_inithash(&s);
  ascon_absorb(&s, in, inlen);
  ascon_squeeze(&s, out, outlen);
  return 0;
}

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long inlen) {
  return ascon_xof(out, CRYPTO_BYTES, in, inlen);
}

#endif
