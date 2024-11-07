
#include "shares.h"

#include <stdlib.h>
#include <string.h>

#include "forceinline.h"
#include "interleave.h"
#include "lendian.h"
#include "randombytes.h"

forceinline uint32_t ROR32(uint32_t x, int n) {
  return x >> n | x << (-n & 31);
}

forceinline uint64_t ROR64(uint64_t x, int n) {
  return x >> n | x << (-n & 63);
}

void generate_shares(uint32_t* s, int num_shares, const uint8_t* data,
                     uint64_t len);

void combine_shares(uint8_t* data, uint64_t len, const uint32_t* s,
                    int num_shares);

void generate_shares_encrypt(const unsigned char* m, mask_m_uint32_t* ms,
                             const unsigned long long mlen,
                             const unsigned char* ad, mask_ad_uint32_t* ads,
                             const unsigned long long adlen,
                             const unsigned char* npub,
                             mask_npub_uint32_t* npubs, const unsigned char* k,
                             mask_key_uint32_t* ks) {
  generate_shares((uint32_t*)ks, NUM_SHARES_KEY, k, CRYPTO_KEYBYTES);
  generate_shares((uint32_t*)npubs, NUM_SHARES_NPUB, npub, CRYPTO_NPUBBYTES);
  generate_shares((uint32_t*)ads, NUM_SHARES_AD, ad, adlen);
  generate_shares((uint32_t*)ms, NUM_SHARES_M, m, mlen);
}

void generate_shares_decrypt(const unsigned char* c, mask_c_uint32_t* cs,
                             const unsigned long long clen,
                             const unsigned char* ad, mask_ad_uint32_t* ads,
                             const unsigned long long adlen,
                             const unsigned char* npub,
                             mask_npub_uint32_t* npubs, const unsigned char* k,
                             mask_key_uint32_t* ks) {
  unsigned long long mlen = clen - CRYPTO_ABYTES;
  mask_c_uint32_t* ts = cs + NUM_WORDS(mlen);
  generate_shares((uint32_t*)ks, NUM_SHARES_KEY, k, CRYPTO_KEYBYTES);
  generate_shares((uint32_t*)npubs, NUM_SHARES_NPUB, npub, CRYPTO_NPUBBYTES);
  generate_shares((uint32_t*)ads, NUM_SHARES_AD, ad, adlen);
  generate_shares((uint32_t*)cs, NUM_SHARES_C, c, mlen);
  generate_shares((uint32_t*)ts, NUM_SHARES_C, c + mlen, CRYPTO_ABYTES);
}

void combine_shares_encrypt(const mask_c_uint32_t* cs, unsigned char* c,
                            unsigned long long clen) {
  unsigned long long mlen = clen - CRYPTO_ABYTES;
  const mask_c_uint32_t* ts = cs + NUM_WORDS(mlen);
  combine_shares(c, mlen, (uint32_t*)cs, NUM_SHARES_C);
  combine_shares(c + mlen, CRYPTO_ABYTES, (uint32_t*)ts, NUM_SHARES_C);
}

void combine_shares_decrypt(const mask_m_uint32_t* ms, unsigned char* m,
                            unsigned long long mlen) {
  combine_shares(m, mlen, (uint32_t*)ms, NUM_SHARES_M);
}

void generate_shares(uint32_t* s, int num_shares, const uint8_t* data,
                     uint64_t len) {
  uint64_t rnd, i;
  /* generate random shares */
  for (i = 0; i < NUM_WORDS(len); i += 2) {
    s[(i + 0) * num_shares + 0] = 0;
    s[(i + 1) * num_shares + 0] = 0;
    for (int d = 1; d < num_shares; ++d) {
      randombytes((uint8_t*)&rnd, 8);
      s[(i + 0) * num_shares + d] = (uint32_t)rnd;
      s[(i + 1) * num_shares + d] = (uint32_t)(rnd >> 32);
#if ASCON_EXTERN_BI
      s[(i + 0) * num_shares + 0] ^= ROR32((uint32_t)rnd, ROT(d));
      s[(i + 1) * num_shares + 0] ^= ROR32((uint32_t)(rnd >> 32), ROT(d));
#else
      rnd = ROR64(rnd, ROT(2 * d));
      s[(i + 0) * num_shares + 0] ^= (uint32_t)rnd;
      s[(i + 1) * num_shares + 0] ^= (uint32_t)(rnd >> 32);
#endif
    }
  }
  /* mask complete words */
  for (i = 0; i < len / 8; ++i) {
    uint64_t x;
    memcpy(&x, data + i * 8, 8);
    x = U64LE(x);
#if ASCON_EXTERN_BI
    x = TOBI(x);
#endif
    s[(2 * i + 0) * num_shares + 0] ^= (uint32_t)x;
    s[(2 * i + 1) * num_shares + 0] ^= (uint32_t)(x >> 32);
  }
  /* mask remaining bytes */
  if ((len / 8 * 8) != len) {
    uint64_t x = 0;
    for (i = (len / 8) * 8; i < len; ++i) {
      x ^= (uint64_t)data[i] << ((i % 8) * 8);
    }
    x = U64LE(x);
#if ASCON_EXTERN_BI
    x = TOBI(x);
#endif
    s[(2 * (len / 8) + 0) * num_shares + 0] ^= (uint32_t)x;
    s[(2 * (len / 8) + 1) * num_shares + 0] ^= (uint32_t)(x >> 32);
  }
}

void combine_shares(uint8_t* data, uint64_t len, const uint32_t* s,
                    int num_shares) {
  uint32_t rnd0, rnd1;
  uint64_t i;
  /* unmask complete words */
  for (i = 0; i < len / 8; ++i) {
    uint64_t x = 0;
    for (int d = 0; d < num_shares; ++d) {
      rnd0 = s[(2 * i + 0) * num_shares + d];
      rnd1 = s[(2 * i + 1) * num_shares + d];
#if ASCON_EXTERN_BI
      x ^= (uint64_t)ROR32(rnd0, ROT(d));
      x ^= (uint64_t)ROR32(rnd1, ROT(d)) << 32;
#else
      x ^= ROR64((uint64_t)rnd1 << 32 | rnd0, ROT(2 * d));
#endif
    }
#if ASCON_EXTERN_BI
    x = FROMBI(x);
#endif
    x = U64LE(x);
    memcpy(data + i * 8, &x, 8);
  }
  /* unmask remaining bytes */
  if ((len / 8 * 8) != len) {
    uint64_t x = 0;
    for (int d = 0; d < num_shares; ++d) {
      rnd0 = s[(2 * (len / 8) + 0) * num_shares + d];
      rnd1 = s[(2 * (len / 8) + 1) * num_shares + d];
#if ASCON_EXTERN_BI
      x ^= (uint64_t)ROR32(rnd0, ROT(d));
      x ^= (uint64_t)ROR32(rnd1, ROT(d)) << 32;
#else
      x ^= ROR64((uint64_t)rnd1 << 32 | rnd0, ROT(2 * d));
#endif
    }
#if ASCON_EXTERN_BI
    x = FROMBI(x);
#endif
    x = U64LE(x);
    for (i = (len / 8) * 8; i < len; ++i) {
      data[i] = x >> ((i % 8) * 8);
    }
  }
}
