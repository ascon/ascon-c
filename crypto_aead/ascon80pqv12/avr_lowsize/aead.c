#include "api.h"
#include "ascon.h"
#include "crypto_aead.h"
#include "permutations.h"
#include "printstate.h"

#if !ASCON_INLINE_MODE
#undef forceinline
#define forceinline
#endif

#ifdef ASCON_AEAD_RATE

forceinline void ascon_loadkey(ascon_key_t* key, const uint8_t* k) {
#if CRYPTO_KEYBYTES == 16
  INSERT(key->b[0], k, 8);
  INSERT(key->b[1], k + 8, 8);
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
  memcpy(s->b[1], key->b[0], 16);
#else /* CRYPTO_KEYBYTES == 20 */
  s->x[0] = key->x[0] ^ ASCON_80PQ_IV;
  memcpy(s->b[1], key->b[1], 16);
#endif
  INSERT(s->b[3], npub, 8);
  INSERT(s->b[4], npub + 8, 8);
  printstate("init 1st key xor", s);
  P(s, 12);
#if CRYPTO_KEYBYTES == 16
  memxor(s->b[3], key->b[0], 16);
#else /* CRYPTO_KEYBYTES == 20 */
  memxor(s->b[2], key->b[0], 24);
#endif
  printstate("init 2nd key xor", s);
}

forceinline void ascon_final(ascon_state_t* s, const ascon_key_t* key) {
#if CRYPTO_KEYBYTES == 16
  memxor(s->b[ASCON_AEAD_RATE / 8], key->b[0], 16);
#else /* CRYPTO_KEYBYTES == 20 */
  s->x[1] ^= KEYROT(key->x[0], key->x[1]);
  s->x[2] ^= KEYROT(key->x[1], key->x[2]);
  s->x[3] ^= KEYROT(key->x[2], 0);
#endif
  printstate("final 1st key xor", s);
  P(s, 12);
#if CRYPTO_KEYBYTES == 16
  memxor(s->b[3], key->b[0], 16);
#else /* CRYPTO_KEYBYTES == 20 */
  memxor(s->b[3], key->b[1], 16);
#endif
  printstate("final 2nd key xor", s);
}

int crypto_aead_encrypt(unsigned char* c, unsigned long long* clen,
                        const unsigned char* m, unsigned long long mlen,
                        const unsigned char* ad, unsigned long long adlen,
                        const unsigned char* nsec, const unsigned char* npub,
                        const unsigned char* k) {
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  ascon_state_t s;
  (void)nsec;
  *clen = mlen + CRYPTO_ABYTES;
  /* perform ascon computation */
  ascon_key_t key;
  ascon_loadkey(&key, k);
  ascon_initaead(&s, &key, npub);
  if (adlen) {
    ascon_update(1, &s, (void*)0, ad, adlen);
    P(&s, nr);
  }
  s.b[4][0] ^= 1;
  printstate("domain separation", &s);
  ascon_update(3, &s, c, m, mlen);
  ascon_final(&s, &key);
  /* set tag */
  SQUEEZE(c + mlen, s.b[3], 8);
  SQUEEZE(c + mlen + 8, s.b[4], 8);
  return 0;
}

int crypto_aead_decrypt(unsigned char* m, unsigned long long* mlen,
                        unsigned char* nsec, const unsigned char* c,
                        unsigned long long clen, const unsigned char* ad,
                        unsigned long long adlen, const unsigned char* npub,
                        const unsigned char* k) {
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  ascon_state_t s;
  (void)nsec;
  if (clen < CRYPTO_ABYTES) return -1;
  *mlen = clen = clen - CRYPTO_ABYTES;
  /* perform ascon computation */
  ascon_key_t key;
  ascon_loadkey(&key, k);
  ascon_initaead(&s, &key, npub);
  if (adlen) {
    ascon_update(1, &s, (void*)0, ad, adlen);
    P(&s, nr);
  }
  s.b[4][0] ^= 1;
  printstate("domain separation", &s);
  ascon_update(7, &s, m, c, clen);
  ascon_final(&s, &key);
  /* verify tag (should be constant time, check compiler output) */
  uint8_t r = 0;
  r |= VERIFY(s.b[3], c + clen, 8);
  r |= VERIFY(s.b[4], c + clen + 8, 8);
  return ((((int)r - 1) >> 8) & 1) - 1;
}

#endif
