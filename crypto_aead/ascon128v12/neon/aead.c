#include "api.h"
#include "ascon.h"
#include "crypto_aead.h"
#include "permutations.h"
#include "printstate.h"

#define AD(NR, RATE, RS, RA)                     \
  do {                                           \
    uint32_t adlen_hi = (uint32_t)(adlen >> 32); \
    uint32_t adlen_lo = (uint32_t)adlen;         \
    __asm__ __volatile__ ( \
        ".arm \n\t" \
        ".fpu neon \n\t" \
        "cmp %[adlen_hi], #0 \n\t" \
        "cmpeq %[adlen_lo], #(%c[R]-1) \n\t" \
        "bls .LAD1 \n\t" \
        "vldm %[s], {d0-d4} \n\t" \
        ".LAD0: \n\t" \
        "vldm %[ad]!, {" RA "} \n\t" \
        "vrev64.8 " RA ", " RA " \n\t" \
        "veor " RS ", " RS ", " RA " \n\t" \
        "vmvn d2, d2 \n\t" \
        P ## NR ## ROUNDS(s) \
        "vmvn d2, d2 \n\t" \
        "sub %[adlen_lo], %[adlen_lo], #%c[R] \n\t" \
        "sbc %[adlen_hi], %[adlen_hi], #0 \n\t" \
        "cmp %[adlen_hi], #0 \n\t" \
        "cmpeq %[adlen_lo], #(%c[R]-1) \n\t" \
        "bhi .LAD0 \n\t" \
        "vstm %[s], {d0-d4} \n\t" \
        ".LAD1: \n\t" \
        : [adlen_hi] "+r" (adlen_hi), [adlen_lo] "+r" (adlen_lo), \
          [ad] "+r" (ad) \
        : [s] "r" (s), [C] "r" (C), [R] "i" (RATE) \
        : "d0", "d1", "d2", "d3", "d4", \
          "d10", "d11", "d12", "d13", "d14", "d16", "d17", \
          "d20", "d21", "d22", "d23", "d24", \
          "d31", "memory");                     \
    adlen = (uint64_t)adlen_hi << 32 | adlen_lo; \
  } while (0)

#define PT(NR, RATE, RS, RM, RC)               \
  do {                                         \
    uint32_t mlen_hi = (uint32_t)(mlen >> 32); \
    uint32_t mlen_lo = (uint32_t)mlen;         \
    __asm__ __volatile__ ( \
        ".arm \n\t" \
        ".fpu neon \n\t" \
        "cmp %[mlen_hi], #0 \n\t" \
        "cmpeq %[mlen_lo], #(%c[R]-1) \n\t" \
        "bls .LPT1 \n\t" \
        "vldm %[s], {d0-d4} \n\t" \
        ".LPT0: \n\t" \
        "vldm %[m]!, {" RM "} \n\t" \
        "vrev64.8 " RM ", " RM " \n\t" \
        "veor " RS ", " RS ", " RM " \n\t" \
        "vrev64.8 " RC ", " RS " \n\t" \
        "vstm %[c]!, {" RC "} \n\t" \
        "vmvn d2, d2 \n\t" \
        P ## NR ## ROUNDS(s) \
        "vmvn d2, d2 \n\t" \
        "sub %[mlen_lo], %[mlen_lo], #%c[R] \n\t" \
        "sbc %[mlen_hi], %[mlen_hi], #0 \n\t" \
        "cmp %[mlen_hi], #0 \n\t" \
        "cmpeq %[mlen_lo], #(%c[R]-1) \n\t" \
        "bhi .LPT0 \n\t" \
        "vstm %[s], {d0-d4} \n\t" \
        ".LPT1: \n\t" \
        : [mlen_hi] "+r" (mlen_hi), [mlen_lo] "+r" (mlen_lo), \
          [m] "+r" (m), [c] "+r" (c) \
        : [s] "r" (s), [C] "r" (C), [R] "i" (RATE) \
        : "d0", "d1", "d2", "d3", "d4", \
          "d10", "d11", "d12", "d13", "d14", "d16", "d17", \
          "d20", "d21", "d22", "d23", "d24", "d26", "d27", \
          "d31", "memory");                   \
    mlen = (uint64_t)mlen_hi << 32 | mlen_lo;  \
  } while (0)

#define CT(NR, RATE, RS, RM, RC)               \
  do {                                         \
    uint32_t clen_hi = (uint32_t)(clen >> 32); \
    uint32_t clen_lo = (uint32_t)clen;         \
    __asm__ __volatile__ ( \
        ".arm \n\t" \
        ".fpu neon \n\t" \
        "cmp %[clen_hi], #0 \n\t" \
        "cmpeq %[clen_lo], #(%c[R]-1) \n\t" \
        "bls .LCT1 \n\t" \
        "vldm %[s], {d0-d4} \n\t" \
        ".LCT0: \n\t" \
        "vldm %[c]!, {" RC "} \n\t" \
        "vrev64.8 " RM ", " RS " \n\t" \
        "veor " RM ", " RM ", " RC " \n\t" \
        "vrev64.8 " RS ", " RC " \n\t" \
        "vstm %[m]!, {" RM "} \n\t" \
        "vmvn d2, d2 \n\t" \
        P ## NR ## ROUNDS(s) \
        "vmvn d2, d2 \n\t" \
        "sub %[clen_lo], %[clen_lo], #%c[R] \n\t" \
        "sbc %[clen_hi], %[clen_hi], #0 \n\t" \
        "cmp %[clen_hi], #0 \n\t" \
        "cmpeq %[clen_lo], #(%c[R]-1) \n\t" \
        "bhi .LCT0 \n\t" \
        "vstm %[s], {d0-d4} \n\t" \
        ".LCT1: \n\t" \
        : [clen_hi] "+r" (clen_hi), [clen_lo] "+r" (clen_lo), \
          [m] "+r" (m), [c] "+r" (c) \
        : [s] "r" (s), [C] "r" (C), [R] "i" (RATE) \
        : "d0", "d1", "d2", "d3", "d4", \
          "d10", "d11", "d12", "d13", "d14", "d16", "d17", \
          "d20", "d21", "d22", "d23", "d24", "d26", "d27", \
          "d31", "memory");                   \
    clen = (uint64_t)clen_hi << 32 | clen_lo;  \
  } while (0)

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
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  if (adlen) {
    /* full associated data blocks */
#if ASCON_AEAD_RATE == 8
    AD(6, 8, "d0", "d16");
#else
    AD(8, 16, "q0", "q8");
#endif
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
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  /* full plaintext blocks */
#if ASCON_AEAD_RATE == 8
  PT(6, 8, "d0", "d16", "d26");
#else
  PT(8, 16, "q0", "q8", "q13");
#endif
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
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  /* full ciphertext blocks */
#if ASCON_AEAD_RATE == 8
  CT(6, 8, "d0", "d16", "d26");
#else
  CT(8, 16, "q0", "q8", "q13");
#endif
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
