#include "api.h"
#include "endian.h"
#include "permutations.h"

#define RATE (64 / 8)
#define PA_ROUNDS 12
#define PB_ROUNDS 6
#define IV                                                        \
  ((u64)(8 * (CRYPTO_KEYBYTES)) << 56 | (u64)(8 * (RATE)) << 48 | \
   (u64)(PA_ROUNDS) << 40 | (u64)(PB_ROUNDS) << 32)

int crypto_aead_decrypt(unsigned char* m, unsigned long long* mlen,
                        unsigned char* nsec, const unsigned char* c,
                        unsigned long long clen, const unsigned char* ad,
                        unsigned long long adlen, const unsigned char* npub,
                        const unsigned char* k) {
  if (clen < CRYPTO_ABYTES) {
    *mlen = 0;
    return -1;
  }

  const u64 K0 = U64BIG(*(u64*)(k + 0)) >> 32;
  const u64 K1 = U64BIG(*(u64*)(k + 4));
  const u64 K2 = U64BIG(*(u64*)(k + 12));
  const u64 N0 = U64BIG(*(u64*)npub);
  const u64 N1 = U64BIG(*(u64*)(npub + 8));
  state s;
  u64 i;
  (void)nsec;

  // set plaintext size
  *mlen = clen - CRYPTO_ABYTES;

  // initialization
  s.x0 = IV | K0;
  s.x1 = K1;
  s.x2 = K2;
  s.x3 = N0;
  s.x4 = N1;
  P12();
  s.x2 ^= K0;
  s.x3 ^= K1;
  s.x4 ^= K2;

  // process associated data
  if (adlen) {
    while (adlen >= RATE) {
      s.x0 ^= U64BIG(*(u64*)ad);
      P6();
      adlen -= RATE;
      ad += RATE;
    }
    for (i = 0; i < adlen; ++i, ++ad) s.x0 ^= INS_BYTE64(*ad, i);
    s.x0 ^= INS_BYTE64(0x80, adlen);
    P6();
  }
  s.x4 ^= 1;

  // process plaintext
  clen -= CRYPTO_ABYTES;
  while (clen >= RATE) {
    *(u64*)m = U64BIG(s.x0) ^ *(u64*)c;
    s.x0 = U64BIG(*((u64*)c));
    P6();
    clen -= RATE;
    m += RATE;
    c += RATE;
  }
  for (i = 0; i < clen; ++i, ++m, ++c) {
    *m = EXT_BYTE64(s.x0, i) ^ *c;
    s.x0 &= ~INS_BYTE64(0xff, i);
    s.x0 |= INS_BYTE64(*c, i);
  }
  s.x0 ^= INS_BYTE64(0x80, clen);

  // finalization
  s.x1 ^= K0 << 32 | K1 >> 32;
  s.x2 ^= K1 << 32 | K2 >> 32;
  s.x3 ^= K2 << 32;
  P12();
  s.x3 ^= K1;
  s.x4 ^= K2;

  // verify tag (should be constant time, check compiler output)
  if (((s.x3 ^ U64BIG(*(u64*)c)) | (s.x4 ^ U64BIG(*(u64*)(c + 8)))) != 0) {
    *mlen = 0;
    return -1;
  }

  return 0;
}

