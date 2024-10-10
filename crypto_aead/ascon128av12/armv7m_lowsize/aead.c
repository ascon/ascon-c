#include "api.h"
#include "ascon.h"
#include "permutations.h"
#include "printstate.h"

#if !ASCON_INLINE_MODE
#undef forceinline
#define forceinline
#endif

#ifdef ASCON_AEAD_RATE

forceinline void ascon_loadkey(ascon_key_t* key, const uint8_t* k) {
#if CRYPTO_KEYBYTES == 16
  key->x[0] = LOAD(k, 8);
  key->x[1] = LOAD(k + 8, 8);
#else /* CRYPTO_KEYBYTES == 20 */
  key->x[0] = KEYROT(0, LOADBYTES(k, 4));
  key->x[1] = LOADBYTES(k + 4, 8);
  key->x[2] = LOADBYTES(k + 12, 8);
#endif
}

forceinline void ascon_initaead(ascon_state_t* s, const ascon_key_t* key,
                                const uint8_t* npub) {
#if CRYPTO_KEYBYTES == 16
  if (ASCON_AEAD_RATE == 8) s->x[0] = ASCON_128_IV;
  if (ASCON_AEAD_RATE == 16) s->x[0] = ASCON_128A_IV;
  s->x[1] = key->x[0];
  s->x[2] = key->x[1];
#else /* CRYPTO_KEYBYTES == 20 */
  s->x[0] = key->x[0] | ASCON_80PQ_IV;
  s->x[1] = key->x[1];
  s->x[2] = key->x[2];
#endif
  s->x[3] = LOAD(npub, 8);
  s->x[4] = LOAD(npub + 8, 8);
  printstate("init 1st key xor", s);
  P(s, 12);
#if CRYPTO_KEYBYTES == 16
  s->x[3] ^= key->x[0];
  s->x[4] ^= key->x[1];
#else /* CRYPTO_KEYBYTES == 20 */
  s->x[2] ^= key->x[0];
  s->x[3] ^= key->x[1];
  s->x[4] ^= key->x[2];
#endif
  printstate("init 2nd key xor", s);
}

forceinline void ascon_final(ascon_state_t* s, const ascon_key_t* key) {
#if CRYPTO_KEYBYTES == 16
  if (ASCON_AEAD_RATE == 8) {
    s->x[1] ^= key->x[0];
    s->x[2] ^= key->x[1];
  } else {
    s->x[2] ^= key->x[0];
    s->x[3] ^= key->x[1];
  }
#else /* CRYPTO_KEYBYTES == 20 */
  s->x[1] ^= KEYROT(key->x[0], key->x[1]);
  s->x[2] ^= KEYROT(key->x[1], key->x[2]);
  s->x[3] ^= KEYROT(key->x[2], 0);
#endif
  printstate("final 1st key xor", s);
  P(s, 12);
#if CRYPTO_KEYBYTES == 16
  s->x[3] ^= key->x[0];
  s->x[4] ^= key->x[1];
#else /* CRYPTO_KEYBYTES == 20 */
  s->x[3] ^= key->x[1];
  s->x[4] ^= key->x[2];
#endif
  printstate("final 2nd key xor", s);
}

int ascon_aead(uint8_t* tag, uint8_t* out, const uint8_t* in, uint64_t inlen,
               const uint8_t* ad, uint64_t adlen, const uint8_t* npub,
               const uint8_t* k, uint8_t mode) {
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  ascon_key_t key;
  ascon_loadkey(&key, k);
  /* initialize */
  ascon_state_t s;
  ascon_initaead(&s, &key, npub);
  /* process associated data */
  if (adlen) {
    ascon_update(&s, (void*)0, ad, adlen, ASCON_ABSORB);
    printstate("pad adata", &s);
    P(&s, nr);
  }
  /* domain separation */
  s.x[4] ^= DSEP();
  printstate("domain separation", &s);
  /* process plaintext/ciphertext */
  ascon_update(&s, out, in, inlen, mode);
  if (mode == ASCON_ENCRYPT) printstate("pad plaintext", &s);
  if (mode == ASCON_DECRYPT) printstate("pad ciphertext", &s);
  /* finalize */
  ascon_final(&s, &key);
  uint8_t t[16];
  ((uint64_t*)t)[0] = WORDTOU64(s.x[3]);
  ((uint64_t*)t)[1] = WORDTOU64(s.x[4]);
  if (mode == ASCON_ENCRYPT) {
    /* get tag */
    for (int i = 0; i < CRYPTO_ABYTES; ++i) tag[i] = t[i];
    return 0;
  }
  /* verify should be constant time, check compiler output */
  int result = 0;
  for (int i = 0; i < CRYPTO_ABYTES; ++i) result |= tag[i] ^ t[i];
  return (((result - 1) >> 8) & 1) - 1;
}

#endif
