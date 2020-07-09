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

  /* initialization */
  s.x0 = 0xee9398aadb67f03dull;
  s.x1 = 0x8bb21831c60f1002ull;
  s.x2 = 0xb48a92db98d5da62ull;
  s.x3 = 0x43189921b8f8e3e8ull;
  s.x4 = 0x348fa5c9d525e140ull;

  /* absorb plaintext */
  while (inlen >= RATE) {
    s.x0 ^= LOAD64(in);
    P(&s, PA_ROUNDS);
    in += RATE;
    inlen -= RATE;
  }
  for (i = 0; i < inlen; ++i, ++in) s.x0 ^= INS_BYTE64(*in, i);
  s.x0 ^= INS_BYTE64(0x80, inlen);

  P(&s, PA_ROUNDS);

  /* absorb plaintext */
  outlen = CRYPTO_BYTES;
  while (outlen > RATE) {
    STORE64(out, s.x0);
    P(&s, PA_ROUNDS);
    out += RATE;
    outlen -= RATE;
  }
  STORE64(out, s.x0);

  return 0;
}
