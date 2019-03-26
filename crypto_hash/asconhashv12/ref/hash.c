#include "api.h"
#include "permutations.h"

#define RATE (64 / 8)
#define PA_ROUNDS 12
#define IV                                            \
  ((u64)(8 * (RATE)) << 48 | (u64)(PA_ROUNDS) << 40 | \
   (u64)(8 * (CRYPTO_BYTES)) << 0)

int crypto_hash(unsigned char *out, const unsigned char *in,
                unsigned long long inlen) {
  state s;
  u64 outlen;

  // initialization
  s.x0 = IV;
  s.x1 = 0;
  s.x2 = 0;
  s.x3 = 0;
  s.x4 = 0;
  printstate("initial value:", s);
  P12(&s);
  printstate("initialization:", s);

  // absorb plaintext
  inlen = inlen;
  while (inlen >= RATE) {
    s.x0 ^= BYTES_TO_U64(in, 8);
    P12(&s);
    inlen -= RATE;
    in += RATE;
  }
  s.x0 ^= BYTES_TO_U64(in, inlen);
  s.x0 ^= 0x80ull << (56 - 8 * inlen);
  printstate("absorb plaintext:", s);

  P12(&s);
  printstate("finalization:", s);

  // set hash output
  outlen = CRYPTO_BYTES;
  while (outlen > RATE) {
    U64_TO_BYTES(out, s.x0, 8);
    P12(&s);
    outlen -= RATE;
    out += RATE;
  }
  U64_TO_BYTES(out, s.x0, 8);

  return 0;
}

