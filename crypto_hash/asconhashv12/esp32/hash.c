#include <string.h>

#include "api.h"
#include "bendian.h"
#include "constants.h"
#include "permutations.h"
#include "printstate.h"

#if ASCON_HASH_BYTES == 32 && ASCON_HASH_ROUNDS == 12
#define IV(i) ASCON_HASH_IV##i
#define PB_START_ROUND 0xf0
#elif ASCON_HASH_BYTES == 32 && ASCON_HASH_ROUNDS == 8
#define IV(i) ASCON_HASHA_IV##i
#define PB_START_ROUND 0xb4
#elif ASCON_HASH_BYTES == 0 && ASCON_HASH_ROUNDS == 12
#define IV(i) ASCON_XOF_IV##i
#define PB_START_ROUND 0xf0
#elif ASCON_HASH_BYTES == 0 && ASCON_HASH_ROUNDS == 8
#define IV(i) ASCON_XOFA_IV##i
#define PB_START_ROUND 0xb4
#endif

#define PA_START_ROUND 0xf0

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long inlen) {
  printbytes("m", in, inlen);
  ascon_state_t s = {0};
  uint32x2_t tmp;
  unsigned long len = inlen;

  // initialization
#ifdef ASCON_PRINT_STATE
  s = (ascon_state_t){{IV(), 0, 0, 0, 0}};
  printstate("initial value", &s);
  P(&s, PA_START_ROUND);
#else
  s = (ascon_state_t){{IV(0), IV(1), IV(2), IV(3), IV(4)}};
#endif
  printstate("initialization", &s);

  while (len >= ASCON_HASH_RATE) {
    tmp.l = ((uint32_t*)in)[0];
    tmp.h = ((uint32_t*)in)[1];
    tmp.x = U64BIG(tmp.x);
    s.w[0].h ^= tmp.h;
    s.w[0].l ^= tmp.l;
    printstate("absorb plaintext", &s);

    P(&s, PB_START_ROUND);

    in += ASCON_HASH_RATE;
    len -= ASCON_HASH_RATE;
  }

  uint8_t* bytes = (uint8_t*)&tmp;
  memset(bytes, 0, sizeof tmp);
  memcpy(bytes, in, len);
  bytes[len] ^= 0x80;

  tmp.x = U64BIG(tmp.x);
  s.w[0].h ^= tmp.h;
  s.w[0].l ^= tmp.l;
  printstate("pad plaintext", &s);

  P(&s, PA_START_ROUND);

  len = CRYPTO_BYTES;
  while (len > ASCON_HASH_RATE) {
    uint32x2_t tmp0 = s.w[0];
    tmp0.x = U64BIG(tmp0.x);
    ((uint32_t*)out)[0] = tmp0.l;
    ((uint32_t*)out)[1] = tmp0.h;
    printstate("squeeze output", &s);

    P(&s, PB_START_ROUND);

    out += ASCON_HASH_RATE;
    len -= ASCON_HASH_RATE;
  }

  tmp = s.w[0];
  tmp.x = U64BIG(tmp.x);
  memcpy(out, bytes, len);
  printstate("squeeze output", &s);
  printbytes("h", out + len - CRYPTO_BYTES, CRYPTO_BYTES);
  return 0;
}
