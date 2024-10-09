#include "api.h"
#include "ascon.h"
#include "constants.h"
#include "permutations.h"
#include "printstate.h"

void ascon_initaead(ascon_state_t* s, const mask_key_uint32_t* k,
                    const mask_npub_uint32_t* n) {
  word_t N0, N1;
  word_t K1, K2;
  /* randomize the initial state */
  s->x[5] = MZERO(NUM_SHARES_KEY);
  s->x[0] = MZERO(NUM_SHARES_KEY);
  /* set the initial value */
  if (ASCON_AEAD_RATE == 8) s->x[0].s[0].w[0] ^= (uint32_t)ASCON_128_IV;
  if (ASCON_AEAD_RATE == 8) s->x[0].s[0].w[1] ^= ASCON_128_IV >> 32;
  if (ASCON_AEAD_RATE == 16) s->x[0].s[0].w[0] ^= (uint32_t)ASCON_128A_IV;
  if (ASCON_AEAD_RATE == 16) s->x[0].s[0].w[1] ^= ASCON_128A_IV >> 32;
  /* set the nonce */
  N0 = MLOAD((uint32_t*)n, NUM_SHARES_NPUB);
  N1 = MLOAD((uint32_t*)(n + 2), NUM_SHARES_NPUB);
  if (NUM_SHARES_KEY == NUM_SHARES_NPUB) {
    s->x[3] = N0;
    s->x[4] = N1;
  } else {
    s->x[3] = MXOR(N0, MZERO(NUM_SHARES_KEY), NUM_SHARES_KEY);
    s->x[4] = MXOR(N1, MZERO(NUM_SHARES_KEY), NUM_SHARES_KEY);
  }
  /* first key xor */
  s->x[1] = K1 = MLOAD((uint32_t*)k, NUM_SHARES_KEY);
  s->x[2] = K2 = MLOAD((uint32_t*)(k + 2), NUM_SHARES_KEY);
  printstate("init 1st key xor", s, NUM_SHARES_KEY);
  /* compute the permutation */
  P(s, ASCON_PA_ROUNDS, NUM_SHARES_KEY);
  /* second key xor */
  s->x[3] = MXOR(s->x[3], K1, NUM_SHARES_KEY);
  s->x[4] = MXOR(s->x[4], K2, NUM_SHARES_KEY);
  printstate("init 2nd key xor", s, NUM_SHARES_KEY);
}

void ascon_adata(ascon_state_t* s, const mask_ad_uint32_t* ad, uint64_t adlen) {
  word_t as;
  const int nr = ASCON_PB_ROUNDS;
  if (adlen) {
    /* full associated data blocks */
    while (adlen >= ASCON_AEAD_RATE) {
      as = MLOAD((uint32_t*)ad, NUM_SHARES_AD);
      s->x[0] = MXOR(s->x[0], as, NUM_SHARES_AD);
      if (ASCON_AEAD_RATE == 16) {
        as = MLOAD((uint32_t*)(ad + 2), NUM_SHARES_AD);
        s->x[1] = MXOR(s->x[1], as, NUM_SHARES_AD);
      }
      printstate("absorb adata", s, NUM_SHARES_AD);
      P(s, nr, NUM_SHARES_AD);
      ad += ASCON_AEAD_RATE / 4;
      adlen -= ASCON_AEAD_RATE;
    }
    /* final associated data block */
    int i = 0;
    if (ASCON_AEAD_RATE == 16 && adlen >= 8) {
      as = MLOAD((uint32_t*)ad, NUM_SHARES_AD);
      s->x[0] = MXOR(s->x[0], as, NUM_SHARES_AD);
      i = 1;
      ad += 2;
      adlen -= 8;
    }
    s->x[i].s[0].w[0] ^= 0x01 << (adlen * 4);
    if (adlen) {
      as = MLOAD((uint32_t*)ad, NUM_SHARES_AD);
      s->x[i] = MXOR(s->x[i], as, NUM_SHARES_AD);
    }
    printstate("pad adata", s, NUM_SHARES_AD);
    P(s, nr, NUM_SHARES_AD);
  }
  /* domain separation */
  s->x[4].s[0].w[1] ^= 0x80000000;
  printstate("domain separation", s, NUM_SHARES_AD);
}

void ascon_encrypt(ascon_state_t* s, mask_c_uint32_t* c,
                   const mask_m_uint32_t* m, uint64_t mlen) {
  word_t ms, cx;
  const int nr = ASCON_PB_ROUNDS;
  /* full plaintext blocks */
  while (mlen >= ASCON_AEAD_RATE) {
    ms = MLOAD((uint32_t*)m, NUM_SHARES_M);
    s->x[0] = MXOR(s->x[0], ms, NUM_SHARES_M);
    cx = MREDUCE(s->x[0], NUM_SHARES_M, NUM_SHARES_C);
    MSTORE((uint32_t*)c, cx, NUM_SHARES_C);
    if (ASCON_AEAD_RATE == 16) {
      ms = MLOAD((uint32_t*)(m + 2), NUM_SHARES_M);
      s->x[1] = MXOR(s->x[1], ms, NUM_SHARES_M);
      cx = MREDUCE(s->x[1], NUM_SHARES_M, NUM_SHARES_C);
      MSTORE((uint32_t*)(c + 2), cx, NUM_SHARES_C);
    }
    printstate("absorb plaintext", s, NUM_SHARES_M);
    P(s, nr, NUM_SHARES_M);
    m += ASCON_AEAD_RATE / 4;
    c += ASCON_AEAD_RATE / 4;
    mlen -= ASCON_AEAD_RATE;
  }
  /* final plaintext block */
  int i = 0;
  if (ASCON_AEAD_RATE == 16 && mlen >= 8) {
    ms = MLOAD((uint32_t*)m, NUM_SHARES_M);
    s->x[0] = MXOR(s->x[0], ms, NUM_SHARES_M);
    cx = MREDUCE(s->x[0], NUM_SHARES_M, NUM_SHARES_C);
    MSTORE((uint32_t*)c, cx, NUM_SHARES_C);
    i = 1;
    m += 2;
    c += 2;
    mlen -= 8;
  }
  s->x[i].s[0].w[0] ^= 0x01 << (mlen * 4);
  if (mlen) {
    ms = MLOAD((uint32_t*)m, NUM_SHARES_M);
    s->x[i] = MXOR(s->x[i], ms, NUM_SHARES_M);
    cx = MREDUCE(s->x[i], NUM_SHARES_M, NUM_SHARES_C);
    MSTORE((uint32_t*)c, cx, NUM_SHARES_C);
  }
  printstate("pad plaintext", s, NUM_SHARES_M);
}

void ascon_decrypt(ascon_state_t* s, mask_m_uint32_t* m,
                   const mask_c_uint32_t* c, uint64_t clen) {
  word_t cx;
  const int nr = ASCON_PB_ROUNDS;
  /* full ciphertext blocks */
  while (clen >= ASCON_AEAD_RATE) {
    cx = MLOAD((uint32_t*)c, NUM_SHARES_C);
    s->x[0] = MXOR(s->x[0], cx, NUM_SHARES_C);
    MSTORE((uint32_t*)m, s->x[0], NUM_SHARES_M);
    s->x[0] = MEXPAND(cx, NUM_SHARES_C, NUM_SHARES_M);
    if (ASCON_AEAD_RATE == 16) {
      cx = MLOAD((uint32_t*)(c + 2), NUM_SHARES_C);
      s->x[1] = MXOR(s->x[1], cx, NUM_SHARES_C);
      MSTORE((uint32_t*)(m + 2), s->x[1], NUM_SHARES_M);
      s->x[1] = MEXPAND(cx, NUM_SHARES_C, NUM_SHARES_M);
    }
    printstate("insert ciphertext", s, NUM_SHARES_M);
    P(s, nr, NUM_SHARES_M);
    m += ASCON_AEAD_RATE / 4;
    c += ASCON_AEAD_RATE / 4;
    clen -= ASCON_AEAD_RATE;
  }
  /* final ciphertext block */
  int i = 0;
  if (ASCON_AEAD_RATE == 16 && clen >= 8) {
    cx = MLOAD((uint32_t*)c, NUM_SHARES_C);
    s->x[0] = MXOR(s->x[0], cx, NUM_SHARES_C);
    MSTORE((uint32_t*)m, s->x[0], NUM_SHARES_M);
    s->x[0] = MEXPAND(cx, NUM_SHARES_C, NUM_SHARES_M);
    i = 1;
    m += 2;
    c += 2;
    clen -= 8;
  }
  s->x[i].s[0].w[0] ^= 0x01 << (clen * 4);
  if (clen) {
    cx = MLOAD((uint32_t*)c, NUM_SHARES_C);
    s->x[i] = MXOR(s->x[i], cx, NUM_SHARES_C);
    MSTORE((uint32_t*)m, s->x[i], NUM_SHARES_M);
    word_t mask = MMASK(s->x[5], clen);
    cx = MEXPAND(cx, NUM_SHARES_C, NUM_SHARES_M);
    s->x[i] = MXORAND(cx, mask, s->x[i], NUM_SHARES_M);
    s->x[5] = MREUSE(s->x[5], 0, NUM_SHARES_M);
  }
  printstate("pad ciphertext", s, NUM_SHARES_M);
}

void ascon_final(ascon_state_t* s, const mask_key_uint32_t* k) {
  word_t K1, K2;
  K1 = MLOAD((uint32_t*)k, NUM_SHARES_KEY);
  K2 = MLOAD((uint32_t*)(k + 2), NUM_SHARES_KEY);
  if (ASCON_AEAD_RATE == 8) {
    /* first key xor (first 64-bit word) */
    s->x[1] = MXOR(s->x[1], K1, NUM_SHARES_KEY);
    /* first key xor (second 64-bit word) */
    s->x[2] = MXOR(s->x[2], K2, NUM_SHARES_KEY);
  } else {
    /* first key xor (first 64-bit word) */
    s->x[2] = MXOR(s->x[2], K1, NUM_SHARES_KEY);
    /* first key xor (second 64-bit word) */
    s->x[3] = MXOR(s->x[3], K2, NUM_SHARES_KEY);
  }
  printstate("final 1st key xor", s, NUM_SHARES_KEY);
  /* compute the permutation */
  P(s, ASCON_PA_ROUNDS, NUM_SHARES_KEY);
  /* second key xor (first 64-bit word) */
  s->x[3] = MXOR(s->x[3], K1, NUM_SHARES_KEY);
  /* second key xor (second 64-bit word) */
  s->x[4] = MXOR(s->x[4], K2, NUM_SHARES_KEY);
  printstate("final 2nd key xor", s, NUM_SHARES_KEY);
}

void ascon_settag(ascon_state_t* s, mask_c_uint32_t* t) {
  s->x[3] = MREDUCE(s->x[3], NUM_SHARES_KEY, NUM_SHARES_C);
  s->x[4] = MREDUCE(s->x[4], NUM_SHARES_KEY, NUM_SHARES_C);
  MSTORE((uint32_t*)t, s->x[3], NUM_SHARES_C);
  MSTORE((uint32_t*)(t + 2), s->x[4], NUM_SHARES_C);
}

/* expected value of x3,x4 for P(0) */
#if ASCON_PB_ROUNDS == 1
static const uint32_t c[4] = {0x4b000009, 0x1c800003, 0x00000000, 0x00000000};
#elif ASCON_PB_ROUNDS == 2
static const uint32_t c[4] = {0x5d2d1034, 0x76fa81d1, 0x0cc1c9ef, 0xdb30a503};
#elif ASCON_PB_ROUNDS == 3
static const uint32_t c[4] = {0xbcaa1d46, 0xf1d0bde9, 0x32c4e651, 0x7b797cd9};
#elif ASCON_PB_ROUNDS == 4
static const uint32_t c[4] = {0xf7820616, 0xeffead2d, 0x94846901, 0xd4895cf5};
#elif ASCON_PB_ROUNDS == 5
static const uint32_t c[4] = {0x9e5ce5e3, 0xd40e9b87, 0x0bfc74af, 0xf8e408a9};
#elif ASCON_PB_ROUNDS == 6
static const uint32_t c[4] = {0x11874f08, 0x7520afef, 0xa4dd41b4, 0x4bd6f9a4};
#else /* ASCON_PB_ROUNDS == 8 */
static const uint32_t c[4] = {0xc7c72006, 0xc3bfab9c, 0x18213f82, 0x0653ed71};
#endif

void ascon_xortag(ascon_state_t* s, const mask_c_uint32_t* t) {
  /* set x0, x1, x2 to zero */
  s->x[0] = MREUSE(s->x[0], 0, NUM_SHARES_KEY);
  s->x[1] = MREUSE(s->x[1], 0, NUM_SHARES_KEY);
  s->x[2] = MREUSE(s->x[2], 0, NUM_SHARES_KEY);
  /* xor tag to x3, x4 */
  word_t t0 = MLOAD((uint32_t*)t, NUM_SHARES_C);
  s->x[3] = MXOR(s->x[3], t0, NUM_SHARES_C);
  word_t t1 = MLOAD((uint32_t*)(t + 2), NUM_SHARES_C);
  s->x[4] = MXOR(s->x[4], t1, NUM_SHARES_C);
#ifndef ASCON_PRINT_STATE
  /* compute P(0) if tags are equal */
  P(s, ASCON_PB_ROUNDS, NUM_SHARES_KEY);
  /* xor expected result to x3, x4 */
  s->x[3].s[0].w[0] ^= c[0];
  s->x[3].s[0].w[1] ^= c[1];
  s->x[4].s[0].w[0] ^= c[2];
  s->x[4].s[0].w[1] ^= c[3];
#endif
}

int ascon_iszero(ascon_state_t* s) {
  s->x[3] = MREDUCE(s->x[3], NUM_SHARES_KEY, 1);
  s->x[4] = MREDUCE(s->x[4], NUM_SHARES_KEY, 1);
  uint32_t result;
  result = s->x[3].s[0].w[0] ^ s->x[3].s[0].w[1];
  result ^= s->x[4].s[0].w[0] ^ s->x[4].s[0].w[1];
  result |= result >> 16;
  result |= result >> 8;
  return ((((int)(result & 0xff) - 1) >> 8) & 1) - 1;
}

#if NUM_SHARES_KEY != NUM_SHARES_AD
void ascon_level_adata(ascon_state_t* s) {
  s->x[0] = MREDUCE(s->x[0], NUM_SHARES_KEY, NUM_SHARES_AD);
  s->x[1] = MREDUCE(s->x[1], NUM_SHARES_KEY, NUM_SHARES_AD);
  s->x[2] = MREDUCE(s->x[2], NUM_SHARES_KEY, NUM_SHARES_AD);
  s->x[3] = MREDUCE(s->x[3], NUM_SHARES_KEY, NUM_SHARES_AD);
  s->x[4] = MREDUCE(s->x[4], NUM_SHARES_KEY, NUM_SHARES_AD);
  s->x[5] = MREDUCE(s->x[5], NUM_SHARES_KEY, NUM_SHARES_AD);
}
#endif

#if NUM_SHARES_AD != NUM_SHARES_M
void ascon_level_encdec(ascon_state_t* s) {
  s->x[0] = MEXPAND(s->x[0], NUM_SHARES_AD, NUM_SHARES_M);
  s->x[1] = MEXPAND(s->x[1], NUM_SHARES_AD, NUM_SHARES_M);
  s->x[2] = MEXPAND(s->x[2], NUM_SHARES_AD, NUM_SHARES_M);
  s->x[3] = MEXPAND(s->x[3], NUM_SHARES_AD, NUM_SHARES_M);
  s->x[4] = MEXPAND(s->x[4], NUM_SHARES_AD, NUM_SHARES_M);
  s->x[5] = MEXPAND(s->x[5], NUM_SHARES_AD, NUM_SHARES_M);
#if NUM_SHARES_M > NUM_SHARES_AD
  s->x[0] = MREFRESH(s->x[0], NUM_SHARES_M);
  s->x[1] = MREFRESH(s->x[1], NUM_SHARES_M);
  s->x[2] = MREFRESH(s->x[2], NUM_SHARES_M);
  s->x[3] = MREFRESH(s->x[3], NUM_SHARES_M);
  s->x[4] = MREFRESH(s->x[4], NUM_SHARES_M);
  s->x[5] = MREFRESH(s->x[5], NUM_SHARES_M);
#endif
}
#endif

#if NUM_SHARES_M != NUM_SHARES_KEY
void ascon_level_final(ascon_state_t* s) {
  s->x[0] = MEXPAND(s->x[0], NUM_SHARES_M, NUM_SHARES_KEY);
  s->x[1] = MEXPAND(s->x[1], NUM_SHARES_M, NUM_SHARES_KEY);
  s->x[2] = MEXPAND(s->x[2], NUM_SHARES_M, NUM_SHARES_KEY);
  s->x[3] = MEXPAND(s->x[3], NUM_SHARES_M, NUM_SHARES_KEY);
  s->x[4] = MEXPAND(s->x[4], NUM_SHARES_M, NUM_SHARES_KEY);
  s->x[5] = MEXPAND(s->x[5], NUM_SHARES_M, NUM_SHARES_KEY);
#if NUM_SHARES_KEY > NUM_SHARES_M
  s->x[0] = MREFRESH(s->x[0], NUM_SHARES_KEY);
  s->x[1] = MREFRESH(s->x[1], NUM_SHARES_KEY);
  s->x[2] = MREFRESH(s->x[2], NUM_SHARES_KEY);
  s->x[3] = MREFRESH(s->x[3], NUM_SHARES_KEY);
  s->x[4] = MREFRESH(s->x[4], NUM_SHARES_KEY);
  s->x[5] = MREFRESH(s->x[5], NUM_SHARES_KEY);
#endif
}
#endif
