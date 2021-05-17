#include "api.h"
#include "ascon.h"
#include "crypto_aead.h"
#include "permutations.h"
#include "printstate.h"

#define AD()                                     \
  do {                                           \
    uint32_t adlen_hi = (uint32_t)(adlen >> 32); \
    uint32_t adlen_lo = (uint32_t)adlen;         \
    __asm__ __volatile__ ( \
        ".arm \n\t" \
        ".fpu neon \n\t" \
        "cmp %[adlen_hi], #0 \n\t" \
        "cmpeq %[adlen_lo], #7 \n\t" \
        "bls .LAD1 \n\t" \
        "vldm %[s], {d0-d4} \n\t" \
        ".LAD0: \n\t" \
        "vldm %[ad]!, {d16} \n\t" \
        "vrev64.8 d16, d16 \n\t" \
        "veor d0, d0, d16 \n\t" \
        "vmvn d2, d2 \n\t" \
        P6ROUNDS(s) \
        "vmvn d2, d2 \n\t" \
        "sub %[adlen_lo], %[adlen_lo], #8 \n\t" \
        "sbc %[adlen_hi], %[adlen_hi], #0 \n\t" \
        "cmp %[adlen_hi], #0 \n\t" \
        "cmpeq %[adlen_lo], #7 \n\t" \
        "bhi .LAD0 \n\t" \
        "vstm %[s], {d0-d4} \n\t" \
        ".LAD1: \n\t" \
        : [adlen_hi] "+r" (adlen_hi), [adlen_lo] "+r" (adlen_lo), \
          [ad] "+r" (ad) \
        : [s] "r" (s), [C] "r" (C) \
        : "d0", "d1", "d2", "d3", "d4", \
          "d10", "d11", "d12", "d13", "d14", "d16", \
          "d20", "d21", "d22", "d23", "d24", \
          "d31", "memory");                     \
    adlen = (uint64_t)adlen_hi << 32 | adlen_lo; \
  } while (0)

#define PT()                                   \
  do {                                         \
    uint32_t mlen_hi = (uint32_t)(mlen >> 32); \
    uint32_t mlen_lo = (uint32_t)mlen;         \
    __asm__ __volatile__ ( \
        ".arm \n\t" \
        ".fpu neon \n\t" \
        "cmp %[mlen_hi], #0 \n\t" \
        "cmpeq %[mlen_lo], #7 \n\t" \
        "bls .LPT1 \n\t" \
        "vldm %[s], {d0-d4} \n\t" \
        ".LPT0: \n\t" \
        "vldm %[m]!, {d16} \n\t" \
        "vrev64.8 d16, d16 \n\t" \
        "veor d0, d0, d16 \n\t" \
        "vrev64.8 d26, d0 \n\t" \
        "vstm %[c]!, {d26} \n\t" \
        "vmvn d2, d2 \n\t" \
        P6ROUNDS(s) \
        "vmvn d2, d2 \n\t" \
        "sub %[mlen_lo], %[mlen_lo], #8 \n\t" \
        "sbc %[mlen_hi], %[mlen_hi], #0 \n\t" \
        "cmp %[mlen_hi], #0 \n\t" \
        "cmpeq %[mlen_lo], #7 \n\t" \
        "bhi .LPT0 \n\t" \
        "vstm %[s], {d0-d4} \n\t" \
        ".LPT1: \n\t" \
        : [mlen_hi] "+r" (mlen_hi), [mlen_lo] "+r" (mlen_lo), \
          [m] "+r" (m), [c] "+r" (c) \
        : [s] "r" (s), [C] "r" (C) \
        : "d0", "d1", "d2", "d3", "d4", \
          "d10", "d11", "d12", "d13", "d14", "d16", \
          "d20", "d21", "d22", "d23", "d24", "d26", \
          "d31", "memory");                   \
    mlen = (uint64_t)mlen_hi << 32 | mlen_lo;  \
  } while (0)

#define CT()                                   \
  do {                                         \
    uint32_t clen_hi = (uint32_t)(clen >> 32); \
    uint32_t clen_lo = (uint32_t)clen;         \
    __asm__ __volatile__ ( \
        ".arm \n\t" \
        ".fpu neon \n\t" \
        "cmp %[clen_hi], #0 \n\t" \
        "cmpeq %[clen_lo], #7 \n\t" \
        "bls .LCT1 \n\t" \
        "vldm %[s], {d0-d4} \n\t" \
        ".LCT0: \n\t" \
        "vldm %[c]!, {d26} \n\t" \
        "vrev64.8 d16, d0 \n\t" \
        "veor d16, d16, d26 \n\t" \
        "vrev64.8 d0, d26 \n\t" \
        "vstm %[m]!, {d16} \n\t" \
        "vmvn d2, d2 \n\t" \
        P6ROUNDS(s) \
        "vmvn d2, d2 \n\t" \
        "sub %[clen_lo], %[clen_lo], #8 \n\t" \
        "sbc %[clen_hi], %[clen_hi], #0 \n\t" \
        "cmp %[clen_hi], #0 \n\t" \
        "cmpeq %[clen_lo], #7 \n\t" \
        "bhi .LCT0 \n\t" \
        "vstm %[s], {d0-d4} \n\t" \
        ".LCT1: \n\t" \
        : [clen_hi] "+r" (clen_hi), [clen_lo] "+r" (clen_lo), \
          [m] "+r" (m), [c] "+r" (c) \
        : [s] "r" (s), [C] "r" (C) \
        : "d0", "d1", "d2", "d3", "d4", \
          "d10", "d11", "d12", "d13", "d14", "d16", \
          "d20", "d21", "d22", "d23", "d24", "d26", \
          "d31", "memory");                   \
    clen = (uint64_t)clen_hi << 32 | clen_lo;  \
  } while (0)

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
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  if (adlen) {
    /* full associated data blocks */
    AD();
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
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  /* full plaintext blocks */
  PT();
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
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  /* full ciphertext blocks */
  CT();
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
