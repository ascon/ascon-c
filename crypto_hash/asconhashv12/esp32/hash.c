#include <string.h>

#include "api.h"
#include "bendian.h"
#include "permutations.h"

#define RATE (64 / 8)
#define PA_ROUNDS 12
#define PA_START_ROUND 0xf0

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long inlen) {
  state s;
  u32_2 tmp;
  unsigned long len = inlen;

  // initialization
  s.x0.h = 0xee9398aa;
  s.x0.l = 0xdb67f03d;
  s.x1.h = 0x8bb21831;
  s.x1.l = 0xc60f1002;
  s.x2.h = 0xb48a92db;
  s.x2.l = 0x98d5da62;
  s.x3.h = 0x43189921;
  s.x3.l = 0xb8f8e3e8;
  s.x4.h = 0x348fa5c9;
  s.x4.l = 0xd525e140;

  while (len >= RATE) {
    tmp.h = ((u32*)in)[0];
    tmp.l = ((u32*)in)[1];
    tmp = ascon_rev8_half(tmp);
    s.x0.h ^= tmp.h;
    s.x0.l ^= tmp.l;

    P(&s, PA_START_ROUND, PA_ROUNDS);

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

    P(&s, PA_START_ROUND, PA_ROUNDS);

    out += RATE;
    len -= RATE;
  }

  tmp = ascon_rev8_half(s.x0);
  memcpy(out, bytes, len);

  return 0;
}
