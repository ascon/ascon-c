#include "api.h"
#include "endian.h"
#include "permutations.h"

#define RATE (64 / 8)
#define PA_ROUNDS 12

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long inlen) {
  state s;
  u64 outlen;
  u64 i;

  // initialization
  s.x0 = 0xb57e273b814cd416ull;
  s.x1 = 0x2b51042562ae2420ull;
  s.x2 = 0x66a3a7768ddf2218ull;
  s.x3 = 0x5aad0a7a8153650cull;
  s.x4 = 0x4f3e0e32539493b6ull;

  // absorb plaintext
  while (inlen >= RATE) {
    s.x0 ^= U64BIG(*(u64*)in);
    P12();
    inlen -= RATE;
    in += RATE;
  }
  for (i = 0; i < inlen; ++i, ++in) s.x0 ^= INS_BYTE64(*in, i);
  s.x0 ^= INS_BYTE64(0x80, inlen);

  P12();

  // absorb plaintext
  outlen = CRYPTO_BYTES;
  while (outlen > RATE) {
    *(u64*)out = U64BIG(s.x0);
    P12();
    outlen -= RATE;
    out += RATE;
  }
  *(u64*)out = U64BIG(s.x0);

  return 0;
}

