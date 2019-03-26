#include "api.h"
#include "endian.h"
#include "permutations.h"

#define RATE (64 / 8)
#define PA_ROUNDS 12

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long inlen) {
  u32_2 x0, x1, x2, x3, x4;
  u32_2 t0, t1, t2, t3, t4;
  u64 outlen, tmp0;
  u32 i;

  // initialization
  x0.e = 0xa540dbc7;
  x0.o = 0xf9afb5c6;
  x1.e = 0x1445a340;
  x1.o = 0xbd249301;
  x2.e = 0x604d4fc8;
  x2.o = 0xcb9ba8b5;
  x3.e = 0x94514c98;
  x3.o = 0x12a4eede;
  x4.e = 0x6339f398;
  x4.o = 0x4bca84c0;

  // absorb plaintext
  while (inlen >= RATE) {
    to_bit_interleaving(t0, U64BIG(*(u64*)in));
    x0.e ^= t0.e;
    x0.o ^= t0.o;
    P12();
    inlen -= RATE;
    in += RATE;
  }
  tmp0 = 0;
  for (i = 0; i < inlen; ++i, ++in) tmp0 |= INS_BYTE64(*in, i);
  tmp0 |= INS_BYTE64(0x80, inlen);
  to_bit_interleaving(t0, tmp0);
  x0.e ^= t0.e;
  x0.o ^= t0.o;

  P12();

  // squeeze output
  outlen = CRYPTO_BYTES;
  while (outlen > RATE) {
    from_bit_interleaving(tmp0, x0);
    *(u64*)out = U64BIG(tmp0);
    P12();
    outlen -= RATE;
    out += RATE;
  }
  from_bit_interleaving(tmp0, x0);
  *(u64*)out = U64BIG(tmp0);

  return 0;
}

