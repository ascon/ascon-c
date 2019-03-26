#include "api.h"
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
  const u64 K0 = BYTES_TO_U64(k, 8) >> 32;
  const u64 K1 = BYTES_TO_U64(k + 4, 8);
  const u64 K2 = BYTES_TO_U64(k + 12, 8);
  const u64 N0 = BYTES_TO_U64(npub, 8);
  const u64 N1 = BYTES_TO_U64(npub + 8, 8);
  state s;
  (void)nsec;

  // set ciphertext size
  *clen = mlen + CRYPTO_ABYTES;

  // initialization
  s.x0 = IV | K0;
  s.x1 = K1;
  s.x2 = K2;
  s.x3 = N0;
  s.x4 = N1;
  printstate("initial value:", s);
  P12(&s);
  s.x2 ^= K0;
  s.x3 ^= K1;
  s.x4 ^= K2;
  printstate("initialization:", s);

  // process associated data
  if (adlen) {
    while (adlen >= RATE) {
      s.x0 ^= BYTES_TO_U64(ad, 8);
      P6(&s);
      adlen -= RATE;
      ad += RATE;
    }
    s.x0 ^= BYTES_TO_U64(ad, adlen);
    s.x0 ^= 0x80ull << (56 - 8 * adlen);
    P6(&s);
  }
  s.x4 ^= 1;
  printstate("process associated data:", s);

  // process plaintext
  while (mlen >= RATE) {
    s.x0 ^= BYTES_TO_U64(m, 8);
    U64_TO_BYTES(c, s.x0, 8);
    P6(&s);
    mlen -= RATE;
    m += RATE;
    c += RATE;
  }
  s.x0 ^= BYTES_TO_U64(m, mlen);
  s.x0 ^= 0x80ull << (56 - 8 * mlen);
  U64_TO_BYTES(c, s.x0, mlen);
  c += mlen;
  printstate("process plaintext:", s);

  // finalization
  s.x1 ^= K0 << 32 | K1 >> 32;
  s.x2 ^= K1 << 32 | K2 >> 32;
  s.x3 ^= K2 << 32;
  P12(&s);
  s.x3 ^= K1;
  s.x4 ^= K2;
  printstate("finalization:", s);

  // set tag
  U64_TO_BYTES(c, s.x3, 8);
  U64_TO_BYTES(c + 8, s.x4, 8);

  return 0;
}

