#include "api.h"
#include "endian.h"
#include "permutations.h"

#define RATE (128 / 8)
#define PA_ROUNDS 12
#define PB_ROUNDS 8
#define IV                                                        \
  ((u64)(8 * (CRYPTO_KEYBYTES)) << 56 | (u64)(8 * (RATE)) << 48 | \
   (u64)(PA_ROUNDS) << 40 | (u64)(PB_ROUNDS) << 32)

int crypto_aead_encrypt(unsigned char* c, unsigned long long* clen,
                        const unsigned char* m, unsigned long long mlen,
                        const unsigned char* ad, unsigned long long adlen,
                        const unsigned char* nsec, const unsigned char* npub,
                        const unsigned char* k) {
  const u64 K0 = U64BIG(*(u64*)k);
  const u64 K1 = U64BIG(*(u64*)(k + 8));
  const u64 N0 = U64BIG(*(u64*)npub);
  const u64 N1 = U64BIG(*(u64*)(npub + 8));
  state s;
  u64 i;
  (void)nsec;

  // set ciphertext size
  *clen = mlen + CRYPTO_ABYTES;

  // initialization
  s.x0 = IV;
  s.x1 = K0;
  s.x2 = K1;
  s.x3 = N0;
  s.x4 = N1;
  P12();
  s.x3 ^= K0;
  s.x4 ^= K1;

  // process associated data
  if (adlen) {
    while (adlen >= RATE) {
      s.x0 ^= U64BIG(*(u64*)ad);
      s.x1 ^= U64BIG(*(u64*)(ad + 8));
      P8();
      adlen -= RATE;
      ad += RATE;
    }
    for (i = 0; i < adlen; ++i, ++ad)
      if (i < 8)
        s.x0 ^= INS_BYTE64(*ad, i);
      else
        s.x1 ^= INS_BYTE64(*ad, i % 8);
    if (adlen < 8)
      s.x0 ^= INS_BYTE64(0x80, adlen);
    else
      s.x1 ^= INS_BYTE64(0x80, adlen % 8);
    P8();
  }
  s.x4 ^= 1;

  // process plaintext
  while (mlen >= RATE) {
    s.x0 ^= U64BIG(*(u64*)m);
    s.x1 ^= U64BIG(*(u64*)(m + 8));
    *(u64*)c = U64BIG(s.x0);
    *(u64*)(c + 8) = U64BIG(s.x1);
    P8();
    mlen -= RATE;
    m += RATE;
    c += RATE;
  }
  for (i = 0; i < mlen; ++i, ++m, ++c) {
    if (i < 8) {
      s.x0 ^= INS_BYTE64(*m, i);
      *c = EXT_BYTE64(s.x0, i);
    } else {
      s.x1 ^= INS_BYTE64(*m, i % 8);
      *c = EXT_BYTE64(s.x1, i % 8);
    }
  }
  if (mlen < 8)
    s.x0 ^= INS_BYTE64(0x80, mlen);
  else
    s.x1 ^= INS_BYTE64(0x80, mlen % 8);

  // finalization
  s.x2 ^= K0;
  s.x3 ^= K1;
  P12();
  s.x3 ^= K0;
  s.x4 ^= K1;

  // set tag
  *(u64*)c = U64BIG(s.x3);
  *(u64*)(c + 8) = U64BIG(s.x4);

  return 0;
}

