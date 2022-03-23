#include "api.h"
#include "ascon.h"
#include "permutations.h"
#include "printstate.h"

#if !ASCON_INLINE_MODE
#undef forceinline
#define forceinline
#endif

#ifdef ASCON_AEAD_RATE

forceinline void ascon_loadkey(key_t* key, const uint8_t* k) {
#if CRYPTO_KEYBYTES == 16
  key->k1 = LOAD(k, 8);
  key->k2 = LOAD(k + 8, 8);
#else /* CRYPTO_KEYBYTES == 20 */
  key->k0 = KEYROT(0, LOADBYTES(k, 4));
  key->k1 = LOADBYTES(k + 4, 8);
  key->k2 = LOADBYTES(k + 12, 8);
#endif
}

forceinline void ascon_initaead(state_t* s, const uint8_t* npub,
                                const key_t* key) {
#if CRYPTO_KEYBYTES == 16
  if (ASCON_AEAD_RATE == 8) s->x[0] = ASCON_128_IV;
  if (ASCON_AEAD_RATE == 16) s->x[0] = ASCON_128A_IV;
#else /* CRYPTO_KEYBYTES == 20 */
  s->x[0] = ASCON_80PQ_IV ^ key->k0;
#endif
  s->x[1] = key->k1;
  s->x[2] = key->k2;
  s->x[3] = LOAD(npub, 8);
  s->x[4] = LOAD(npub + 8, 8);
  printstate("init 1st key xor", s);
  P(s, 12);
#if CRYPTO_KEYBYTES == 20
  s->x[2] ^= key->k0;
#endif
  s->x[3] ^= key->k1;
  s->x[4] ^= key->k2;
  printstate("init 2nd key xor", s);
}

forceinline void ascon_final(state_t* s, const key_t* key) {
#if CRYPTO_KEYBYTES == 16
  if (ASCON_AEAD_RATE == 8) {
    s->x[1] ^= key->k1;
    s->x[2] ^= key->k2;
  } else {
    s->x[2] ^= key->k1;
    s->x[3] ^= key->k2;
  }
#else /* CRYPTO_KEYBYTES == 20 */
  s->x[1] ^= KEYROT(key->k0, key->k1);
  s->x[2] ^= KEYROT(key->k1, key->k2);
  s->x[3] ^= KEYROT(key->k2, 0);
#endif
  printstate("final 1st key xor", s);
  P(s, 12);
  s->x[3] ^= key->k1;
  s->x[4] ^= key->k2;
  printstate("final 2nd key xor", s);
}

void ascon_aead(uint8_t* t, uint8_t* out, const uint8_t* in, uint64_t tlen,
                const uint8_t* ad, uint64_t adlen, const uint8_t* npub,
                const uint8_t* k, uint8_t mode) {
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  key_t key;
  ascon_loadkey(&key, k);
  /* initialize */
  state_t s;
  ascon_initaead(&s, npub, &key);
  /* process associated data */
  if (adlen) {
    ascon_update(&s, (void*)0, ad, adlen, ASCON_ABSORB);
    printstate("pad adata", &s);
    P(&s, nr);
  }
  /* domain separation */
  s.x[4] ^= 1;
  printstate("domain separation", &s);
  /* process plaintext/ciphertext */
  ascon_update(&s, out, in, tlen, mode);
  if (mode == ASCON_ENCRYPT) printstate("pad plaintext", &s);
  if (mode == ASCON_DECRYPT) printstate("pad ciphertext", &s);
  /* finalize */
  ascon_final(&s, &key);
  ((uint64_t*)t)[0] = WORDTOU64(s.x[3]);
  ((uint64_t*)t)[1] = WORDTOU64(s.x[4]);
}

#endif
