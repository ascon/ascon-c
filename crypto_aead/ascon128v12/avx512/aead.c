#include "api.h"
#include "ascon.h"
#include "crypto_aead.h"
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

#ifdef ASCON_AEAD_RATE

forceinline void ascon_loadkey(ascon_key_t* key, const uint8_t* k) {
#if CRYPTO_KEYBYTES == 16
  key->k1 = LOAD(k, 8);
  key->k2 = LOAD(k + 8, 8);
#else /* CRYPTO_KEYBYTES == 20 */
  key->k0 = KEYROT(0, LOADBYTES(k, 4));
  key->k1 = LOADBYTES(k + 4, 8);
  key->k2 = LOADBYTES(k + 12, 8);
#endif
}

forceinline void ascon_initaead(ascon_state_t* s, const ascon_key_t* key,
                                const uint8_t* npub) {
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

forceinline void ascon_adata(ascon_state_t* s, const uint8_t* ad,
                             uint64_t adlen) {
  const __m512i u64big = AVX512_SHUFFLE_U64BIG;
  const int mask = (ASCON_AEAD_RATE == 8) ? 0xff : 0xffff;
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  ascon_state_t r = *s, t;
  if (adlen) {
    /* full associated data blocks */
    while (adlen >= ASCON_AEAD_RATE) {
      t.z = _mm512_maskz_loadu_epi8(mask, ad);
      t.z = _mm512_maskz_shuffle_epi8(mask, t.z, u64big);
      r.z = _mm512_xor_epi64(r.z, t.z);
      printstate("absorb adata", &r);
      P(&r, nr);
      ad += ASCON_AEAD_RATE;
      adlen -= ASCON_AEAD_RATE;
    }
    *s = r;
    /* final associated data block */
    uint64_t* px = &s->x[0];
    if (ASCON_AEAD_RATE == 16 && adlen >= 8) {
      s->x[0] ^= LOAD(ad, 8);
      px = &s->x[1];
      ad += 8;
      adlen -= 8;
    }
    *px ^= PAD(adlen);
    if (adlen) *px ^= LOAD(ad, adlen);
    printstate("pad adata", s);
    P(s, nr);
  }
  /* domain separation */
  s->x[4] ^= 1;
  printstate("domain separation", s);
}

forceinline void ascon_encrypt(ascon_state_t* s, uint8_t* c, const uint8_t* m,
                               uint64_t mlen) {
  const __m512i u64big = AVX512_SHUFFLE_U64BIG;
  const int mask = (ASCON_AEAD_RATE == 8) ? 0xff : 0xffff;
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  ascon_state_t r = *s, t;
  /* full plaintext blocks */
  while (mlen >= ASCON_AEAD_RATE) {
    t.z = _mm512_maskz_loadu_epi8(mask, m);
    t.z = _mm512_maskz_shuffle_epi8(mask, t.z, u64big);
    r.z = _mm512_xor_epi64(r.z, t.z);
    t.z = _mm512_maskz_shuffle_epi8(mask, r.z, u64big);
    _mm512_mask_storeu_epi8(c, mask, t.z);
    printstate("absorb plaintext", &r);
    P(&r, nr);
    m += ASCON_AEAD_RATE;
    c += ASCON_AEAD_RATE;
    mlen -= ASCON_AEAD_RATE;
  }
  *s = r;
  /* final plaintext block */
  uint64_t* px = &s->x[0];
  if (ASCON_AEAD_RATE == 16 && mlen >= 8) {
    s->x[0] ^= LOAD(m, 8);
    STORE(c, s->x[0], 8);
    px = &s->x[1];
    m += 8;
    c += 8;
    mlen -= 8;
  }
  *px ^= PAD(mlen);
  if (mlen) {
    *px ^= LOAD(m, mlen);
    STORE(c, *px, mlen);
  }
  printstate("pad plaintext", s);
}

forceinline void ascon_decrypt(ascon_state_t* s, uint8_t* m, const uint8_t* c,
                               uint64_t clen) {
  const __m512i u64big = AVX512_SHUFFLE_U64BIG;
  const int mask = (ASCON_AEAD_RATE == 8) ? 0xff : 0xffff;
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  ascon_state_t r = *s, t, u;
  /* full ciphertext blocks */
  while (clen >= ASCON_AEAD_RATE) {
    t.z = _mm512_maskz_loadu_epi8(mask, c);
    t.z = _mm512_maskz_shuffle_epi8(mask, t.z, u64big);
    r.z = _mm512_xor_epi64(r.z, t.z);
    u.z = _mm512_maskz_shuffle_epi8(mask, r.z, u64big);
    r.z = _mm512_mask_blend_epi8(mask, r.z, t.z);
    _mm512_mask_storeu_epi8(m, mask, u.z);
    printstate("insert ciphertext", &r);
    P(&r, nr);
    m += ASCON_AEAD_RATE;
    c += ASCON_AEAD_RATE;
    clen -= ASCON_AEAD_RATE;
  }
  *s = r;
  /* final ciphertext block */
  uint64_t* px = &s->x[0];
  if (ASCON_AEAD_RATE == 16 && clen >= 8) {
    uint64_t cx = LOAD(c, 8);
    s->x[0] ^= cx;
    STORE(m, s->x[0], 8);
    s->x[0] = cx;
    px = &s->x[1];
    m += 8;
    c += 8;
    clen -= 8;
  }
  *px ^= PAD(clen);
  if (clen) {
    uint64_t cx = LOAD(c, clen);
    *px ^= cx;
    STORE(m, *px, clen);
    *px = CLEAR(*px, clen);
    *px ^= cx;
  }
  printstate("pad ciphertext", s);
}

forceinline void ascon_final(ascon_state_t* s, const ascon_key_t* key) {
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

int crypto_aead_encrypt(unsigned char* c, unsigned long long* clen,
                        const unsigned char* m, unsigned long long mlen,
                        const unsigned char* ad, unsigned long long adlen,
                        const unsigned char* nsec, const unsigned char* npub,
                        const unsigned char* k) {
  ascon_state_t s;
  (void)nsec;
  *clen = mlen + CRYPTO_ABYTES;
  /* perform ascon computation */
  ascon_key_t key;
  ascon_loadkey(&key, k);
  ascon_initaead(&s, &key, npub);
  ascon_adata(&s, ad, adlen);
  ascon_encrypt(&s, c, m, mlen);
  ascon_final(&s, &key);
  /* set tag */
  STOREBYTES(c + mlen, s.x[3], 8);
  STOREBYTES(c + mlen + 8, s.x[4], 8);
  return 0;
}

int crypto_aead_decrypt(unsigned char* m, unsigned long long* mlen,
                        unsigned char* nsec, const unsigned char* c,
                        unsigned long long clen, const unsigned char* ad,
                        unsigned long long adlen, const unsigned char* npub,
                        const unsigned char* k) {
  ascon_state_t s;
  (void)nsec;
  if (clen < CRYPTO_ABYTES) return -1;
  *mlen = clen = clen - CRYPTO_ABYTES;
  /* perform ascon computation */
  ascon_key_t key;
  ascon_loadkey(&key, k);
  ascon_initaead(&s, &key, npub);
  ascon_adata(&s, ad, adlen);
  ascon_decrypt(&s, m, c, clen);
  ascon_final(&s, &key);
  /* verify tag (should be constant time, check compiler output) */
  s.x[3] ^= LOADBYTES(c + clen, 8);
  s.x[4] ^= LOADBYTES(c + clen + 8, 8);
  return NOTZERO(s.x[3], s.x[4]);
}

#endif
