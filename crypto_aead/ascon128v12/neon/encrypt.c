#include "api.h"
#include "endian.h"
#include "permutations.h"

#define RATE (64 / 8)
#define PA_ROUNDS 12
#define PB_ROUNDS 6
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
  u32 i;
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
    AD();
    for (i = 0; i < adlen; ++i, ++ad) s.x0 ^= INS_BYTE64(*ad, i);
    s.x0 ^= INS_BYTE64(0x80, adlen);
    P6();
  }
  s.x4 ^= 1;

  // process plaintext
  PT();
  for (i = 0; i < mlen; ++i, ++m, ++c) {
    s.x0 ^= INS_BYTE64(*m, i);
    *c = EXT_BYTE64(s.x0, i);
  }
  s.x0 ^= INS_BYTE64(0x80, mlen);

  // finalization
  s.x1 ^= K0;
  s.x2 ^= K1;
  P12();
  s.x3 ^= K0;
  s.x4 ^= K1;

  // set tag
  *(u64*)c = U64BIG(s.x3);
  *(u64*)(c + 8) = U64BIG(s.x4);

  return 0;
}

