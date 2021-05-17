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

forceinline void ascon_loadkey(word_t* K0, word_t* K1, word_t* K2,
                               const uint8_t* k) {
  KINIT(K0, K1, K2);
  if (CRYPTO_KEYBYTES == 16) {
    *K1 = XOR(*K1, LOAD(k, 8));
    *K2 = XOR(*K2, LOAD(k + 8, 8));
  }
  if (CRYPTO_KEYBYTES == 20) {
    *K0 = XOR(*K0, KEYROT(WORD_T(0), LOADBYTES(k, 4)));
    *K1 = XOR(*K1, LOADBYTES(k + 4, 8));
    *K2 = XOR(*K2, LOADBYTES(k + 12, 8));
  }
}

forceinline void ascon_aeadinit(state_t* s, const uint8_t* npub,
                                const uint8_t* k) {
  /* load nonce */
  word_t N0 = LOAD(npub, 8);
  word_t N1 = LOAD(npub + 8, 8);
  /* load key */
  word_t K0, K1, K2;
  ascon_loadkey(&K0, &K1, &K2, k);
  /* initialize */
  PINIT(s);
  if (CRYPTO_KEYBYTES == 16 && ASCON_AEAD_RATE == 8)
    s->x0 = XOR(s->x0, ASCON_128_IV);
  if (CRYPTO_KEYBYTES == 16 && ASCON_AEAD_RATE == 16)
    s->x0 = XOR(s->x0, ASCON_128A_IV);
  if (CRYPTO_KEYBYTES == 20) s->x0 = XOR(s->x0, ASCON_80PQ_IV);
  if (CRYPTO_KEYBYTES == 20) s->x0 = XOR(s->x0, K0);
  s->x1 = XOR(s->x1, K1);
  s->x2 = XOR(s->x2, K2);
  s->x3 = XOR(s->x3, N0);
  s->x4 = XOR(s->x4, N1);
  P(s, 12);
  if (CRYPTO_KEYBYTES == 20) s->x2 = XOR(s->x2, K0);
  s->x3 = XOR(s->x3, K1);
  s->x4 = XOR(s->x4, K2);
  printstate("initialization", s);
}

forceinline void ascon_adata(state_t* s, const uint8_t* ad, uint64_t adlen) {
  const __m512i u64big = AVX512_SHUFFLE_U64BIG;
  const int mask = (ASCON_AEAD_RATE == 8) ? 0xff : 0xffff;
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  state_t r = *s, t;
  if (adlen) {
    /* full associated data blocks */
    while (adlen >= ASCON_AEAD_RATE) {
      t.z = _mm512_maskz_loadu_epi8(mask, ad);
      t.z = _mm512_maskz_shuffle_epi8(mask, t.z, u64big);
      r.z = _mm512_xor_epi64(r.z, t.z);
      P(&r, nr);
      ad += ASCON_AEAD_RATE;
      adlen -= ASCON_AEAD_RATE;
    }
    *s = r;
    /* final associated data block */
    word_t* px = &s->x0;
    if (ASCON_AEAD_RATE == 16 && adlen >= 8) {
      s->x0 = XOR(s->x0, LOAD(ad, 8));
      px = &s->x1;
      ad += 8;
      adlen -= 8;
    }
    *px = XOR(*px, PAD(adlen));
    if (adlen) *px = XOR(*px, LOAD(ad, adlen));
    P(s, nr);
  }
  /* domain separation */
  s->x4 = XOR(s->x4, WORD_T(1));
  printstate("process associated data", s);
}

forceinline void ascon_encrypt(state_t* s, uint8_t* c, const uint8_t* m,
                               uint64_t mlen) {
  const __m512i u64big = AVX512_SHUFFLE_U64BIG;
  const int mask = (ASCON_AEAD_RATE == 8) ? 0xff : 0xffff;
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  state_t r = *s, t;
  /* full plaintext blocks */
  while (mlen >= ASCON_AEAD_RATE) {
    t.z = _mm512_maskz_loadu_epi8(mask, m);
    t.z = _mm512_maskz_shuffle_epi8(mask, t.z, u64big);
    r.z = _mm512_xor_epi64(r.z, t.z);
    t.z = _mm512_maskz_shuffle_epi8(mask, r.z, u64big);
    _mm512_mask_storeu_epi8(c, mask, t.z);
    P(&r, nr);
    m += ASCON_AEAD_RATE;
    c += ASCON_AEAD_RATE;
    mlen -= ASCON_AEAD_RATE;
  }
  *s = r;
  /* final plaintext block */
  word_t* px = &s->x0;
  if (ASCON_AEAD_RATE == 16 && mlen >= 8) {
    s->x0 = XOR(s->x0, LOAD(m, 8));
    STORE(c, s->x0, 8);
    px = &s->x1;
    m += 8;
    c += 8;
    mlen -= 8;
  }
  *px = XOR(*px, PAD(mlen));
  if (mlen) {
    *px = XOR(*px, LOAD(m, mlen));
    STORE(c, *px, mlen);
  }
  printstate("process plaintext", s);
}

forceinline void ascon_decrypt(state_t* s, uint8_t* m, const uint8_t* c,
                               uint64_t clen) {
  const __m512i u64big = AVX512_SHUFFLE_U64BIG;
  const int mask = (ASCON_AEAD_RATE == 8) ? 0xff : 0xffff;
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  state_t r = *s, t, u;
  /* full ciphertext blocks */
  while (clen >= ASCON_AEAD_RATE) {
    t.z = _mm512_maskz_loadu_epi8(mask, c);
    t.z = _mm512_maskz_shuffle_epi8(mask, t.z, u64big);
    r.z = _mm512_xor_epi64(r.z, t.z);
    u.z = _mm512_maskz_shuffle_epi8(mask, r.z, u64big);
    r.z = _mm512_mask_blend_epi8(mask, r.z, t.z);
    _mm512_mask_storeu_epi8(m, mask, u.z);
    P(&r, nr);
    m += ASCON_AEAD_RATE;
    c += ASCON_AEAD_RATE;
    clen -= ASCON_AEAD_RATE;
  }
  *s = r;
  /* final ciphertext block */
  word_t* px = &s->x0;
  if (ASCON_AEAD_RATE == 16 && clen >= 8) {
    word_t cx = LOAD(c, 8);
    s->x0 = XOR(s->x0, cx);
    STORE(m, s->x0, 8);
    s->x0 = cx;
    px = &s->x1;
    m += 8;
    c += 8;
    clen -= 8;
  }
  *px = XOR(*px, PAD(clen));
  if (clen) {
    word_t cx = LOAD(c, clen);
    *px = XOR(*px, cx);
    STORE(m, *px, clen);
    *px = CLEAR(*px, clen);
    *px = XOR(*px, cx);
  }
  printstate("process ciphertext", s);
}

forceinline void ascon_final(state_t* s, const uint8_t* k) {
  /* load key */
  word_t K0, K1, K2;
  ascon_loadkey(&K0, &K1, &K2, k);
  /* finalize */
  if (CRYPTO_KEYBYTES == 16 && ASCON_AEAD_RATE == 8) {
    s->x1 = XOR(s->x1, K1);
    s->x2 = XOR(s->x2, K2);
  }
  if (CRYPTO_KEYBYTES == 16 && ASCON_AEAD_RATE == 16) {
    s->x2 = XOR(s->x2, K1);
    s->x3 = XOR(s->x3, K2);
  }
  if (CRYPTO_KEYBYTES == 20) {
    s->x1 = XOR(s->x1, KEYROT(K0, K1));
    s->x2 = XOR(s->x2, KEYROT(K1, K2));
    s->x3 = XOR(s->x3, KEYROT(K2, WORD_T(0)));
  }
  P(s, 12);
  s->x3 = XOR(s->x3, K1);
  s->x4 = XOR(s->x4, K2);
  printstate("finalization", s);
}

int crypto_aead_encrypt(unsigned char* c, unsigned long long* clen,
                        const unsigned char* m, unsigned long long mlen,
                        const unsigned char* ad, unsigned long long adlen,
                        const unsigned char* nsec, const unsigned char* npub,
                        const unsigned char* k) {
  state_t s;
  (void)nsec;
  *clen = mlen + CRYPTO_ABYTES;
  /* perform ascon computation */
  ascon_aeadinit(&s, npub, k);
  ascon_adata(&s, ad, adlen);
  ascon_encrypt(&s, c, m, mlen);
  ascon_final(&s, k);
  /* set tag */
  STOREBYTES(c + mlen, s.x3, 8);
  STOREBYTES(c + mlen + 8, s.x4, 8);
  return 0;
}

int crypto_aead_decrypt(unsigned char* m, unsigned long long* mlen,
                        unsigned char* nsec, const unsigned char* c,
                        unsigned long long clen, const unsigned char* ad,
                        unsigned long long adlen, const unsigned char* npub,
                        const unsigned char* k) {
  state_t s;
  (void)nsec;
  if (clen < CRYPTO_ABYTES) return -1;
  *mlen = clen = clen - CRYPTO_ABYTES;
  /* perform ascon computation */
  ascon_aeadinit(&s, npub, k);
  ascon_adata(&s, ad, adlen);
  ascon_decrypt(&s, m, c, clen);
  ascon_final(&s, k);
  /* verify tag (should be constant time, check compiler output) */
  s.x3 = XOR(s.x3, LOADBYTES(c + clen, 8));
  s.x4 = XOR(s.x4, LOADBYTES(c + clen + 8, 8));
  return NOTZERO(s.x3, s.x4);
}
