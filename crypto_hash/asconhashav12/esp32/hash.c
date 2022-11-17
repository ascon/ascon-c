#include <string.h>

#include "api.h"
#include "bendian.h"
#include "permutations.h"

#define RATE (64 / 8)
#define PA_ROUNDS 12
#define PA_START_ROUND 0xf0
#define PB_ROUNDS 8
#define PB_START_ROUND 0xb4

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long inlen) {
  state s;
  u32_2 tmp;
  unsigned long len = inlen;

  // initialization
  s.x0.h = 0x01470194;
  s.x0.l = 0xfc6528a6;
  s.x1.h = 0x738ec38a;
  s.x1.l = 0xc0adffa7;
  s.x2.h = 0x2ec8e329;
  s.x2.l = 0x6c76384c;
  s.x3.h = 0xd6f6a54d;
  s.x3.l = 0x7f52377d;
  s.x4.h = 0xa13c42a2;
  s.x4.l = 0x23be8d87;

  while (len >= RATE) {
    tmp.h = ((u32*)in)[0];
    tmp.l = ((u32*)in)[1];
    tmp = ascon_rev8_half(tmp);
    s.x0.h ^= tmp.h;
    s.x0.l ^= tmp.l;

    P(&s, PB_START_ROUND, PB_ROUNDS);

    in += RATE;
    len -= RATE;
  }

  u8* bytes = (u8*)&tmp;
  memset(bytes, 0, sizeof tmp);
  memcpy(bytes, in, len);
  bytes[len] ^= 0x80;

  tmp = ascon_rev8_half(tmp);
  s.x0.h ^= tmp.h;
  s.x0.l ^= tmp.l;

  P(&s, PA_START_ROUND, PA_ROUNDS);

  len = CRYPTO_BYTES;
  while (len >= RATE) {
    ((u32*)out)[0] = U32BIG(s.x0.h);
    ((u32*)out)[1] = U32BIG(s.x0.l);

    P(&s, PB_START_ROUND, PB_ROUNDS);

    out += RATE;
    len -= RATE;
  }

  tmp = ascon_rev8_half(s.x0);
  memcpy(out, bytes, len);

  return 0;
}
