#include "api.h"
#include "endian.h"
#include "permutations.h"

#define RATE (64 / 8)
#define PA_ROUNDS 12

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long inlen) {
  state s;
  u32_2 t0;
  u64 outlen, tmp0;
  u64 i;

  /* initialization */
  s.x0.e = 0xa540dbc7;
  s.x0.o = 0xf9afb5c6;
  s.x1.e = 0x1445a340;
  s.x1.o = 0xbd249301;
  s.x2.e = 0x604d4fc8;
  s.x2.o = 0xcb9ba8b5;
  s.x3.e = 0x94514c98;
  s.x3.o = 0x12a4eede;
  s.x4.e = 0x6339f398;
  s.x4.o = 0x4bca84c0;

  /* absorb plaintext */
  while (inlen >= RATE) {
    tmp0 = LOAD64(in);
    t0 = to_bit_interleaving(tmp0);
    s.x0.e ^= t0.e;
    s.x0.o ^= t0.o;
    P(&s, PA_ROUNDS);
    in += RATE;
    inlen -= RATE;
  }
  tmp0 = 0;
  for (i = 0; i < inlen; ++i, ++in) tmp0 |= INS_BYTE64(*in, i);
  tmp0 |= INS_BYTE64(0x80, inlen);
  t0 = to_bit_interleaving(tmp0);
  s.x0.e ^= t0.e;
  s.x0.o ^= t0.o;

  P(&s, PA_ROUNDS);

  /* squeeze output */
  outlen = CRYPTO_BYTES;
  while (outlen > RATE) {
    tmp0 = from_bit_interleaving(s.x0);
    STORE64(out, tmp0);
    P(&s, PA_ROUNDS);
    outlen -= RATE;
    out += RATE;
  }
  tmp0 = from_bit_interleaving(s.x0);
  STORE64(out, tmp0);

  return 0;
}
