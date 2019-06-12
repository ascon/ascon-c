#include "api.h"
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

  const u64 K0 = BYTES_TO_U64(k, 8);
  const u64 K1 = BYTES_TO_U64(k + 8, 8);
  const u64 N0 = BYTES_TO_U64(npub, 8);
  const u64 N1 = BYTES_TO_U64(npub + 8, 8);
  state s;
  u64 c0;
  (void)nsec;

  // set plaintext size
  *mlen = clen - CRYPTO_ABYTES;

  // initialization
  s.x0 = IV;
  s.x1 = K0;
  s.x2 = K1;
  s.x3 = N0;
  s.x4 = N1;
  printstate("initial value:", s);
  P12(&s);
  s.x3 ^= K0;
  s.x4 ^= K1;
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
  clen -= CRYPTO_ABYTES;
  while (clen >= RATE) {
    c0 = BYTES_TO_U64(c, 8);
    U64_TO_BYTES(m, s.x0 ^ c0, 8);
    s.x0 = c0;
    P6(&s);
    clen -= RATE;
    m += RATE;
    c += RATE;
  }
  c0 = BYTES_TO_U64(c, clen);
  U64_TO_BYTES(m, s.x0 ^ c0, clen);
  s.x0 &= ~BYTE_MASK(clen);
  s.x0 |= c0;
  s.x0 ^= 0x80ull << (56 - 8 * clen);
  c += clen;
  printstate("process plaintext:", s);

  // finalization
  s.x1 ^= K0;
  s.x2 ^= K1;
  P12(&s);
  s.x3 ^= K0;
  s.x4 ^= K1;
  printstate("finalization:", s);

  // verify tag (should be constant time, check compiler output)
  if (((s.x3 ^ BYTES_TO_U64(c, 8)) | (s.x4 ^ BYTES_TO_U64(c + 8, 8))) != 0) {
    *mlen = 0;
    return -1;
  }

  return 0;
}

