#include "api.h"
#include "endian.h"
#include "permutations.h"

#define RATE (128 / 8)
#define PA_ROUNDS 12
#define PB_ROUNDS 8
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

  u32_2 K0, K1, N0, N1;
  u32_2 x0, x1, x2, x3, x4;
  u32_2 t0, t1, t2, t3, t4;
  u64 tmp0, tmp1;
  u32 i;
  (void)nsec;

  // set plaintext size
  *mlen = clen - CRYPTO_ABYTES;

  to_bit_interleaving(K0, U64BIG(*(u64*)k));
  to_bit_interleaving(K1, U64BIG(*(u64*)(k + 8)));
  to_bit_interleaving(N0, U64BIG(*(u64*)npub));
  to_bit_interleaving(N1, U64BIG(*(u64*)(npub + 8)));

  // initialization
  to_bit_interleaving(x0, IV);
  x1.o = K0.o;
  x1.e = K0.e;
  x2.e = K1.e;
  x2.o = K1.o;
  x3.e = N0.e;
  x3.o = N0.o;
  x4.e = N1.e;
  x4.o = N1.o;
  P12();
  x3.e ^= K0.e;
  x3.o ^= K0.o;
  x4.e ^= K1.e;
  x4.o ^= K1.o;

  // process associated data
  if (adlen) {
    while (adlen >= RATE) {
      to_bit_interleaving(t0, U64BIG(*(u64*)ad));
      x0.e ^= t0.e;
      x0.o ^= t0.o;
      to_bit_interleaving(t1, U64BIG(*(u64*)(ad + 8)));
      x1.e ^= t1.e;
      x1.o ^= t1.o;
      P8();
      adlen -= RATE;
      ad += RATE;
    }
    tmp0 = 0;
    tmp1 = 0;
    for (i = 0; i < adlen; ++i, ++ad)
      if (i < 8)
        tmp0 ^= INS_BYTE64(*ad, i);
      else
        tmp1 ^= INS_BYTE64(*ad, i % 8);
    if (adlen < 8)
      tmp0 ^= INS_BYTE64(0x80, adlen);
    else
      tmp1 ^= INS_BYTE64(0x80, adlen % 8);
    to_bit_interleaving(t0, tmp0);
    x0.e ^= t0.e;
    x0.o ^= t0.o;
    to_bit_interleaving(t1, tmp1);
    x1.e ^= t1.e;
    x1.o ^= t1.o;
    P8();
  }
  x4.e ^= 1;

  // process plaintext
  clen -= CRYPTO_ABYTES;
  while (clen >= RATE) {
    from_bit_interleaving(tmp0, x0);
    from_bit_interleaving(tmp1, x1);
    *(u64*)m = U64BIG(tmp0) ^ *(u64*)c;
    *(u64*)(m + 8) = U64BIG(tmp1) ^ *(u64*)(c + 8);
    to_bit_interleaving(x0, U64BIG(*(u64*)c));
    to_bit_interleaving(x1, U64BIG(*(u64*)(c + 8)));
    P8();
    clen -= RATE;
    m += RATE;
    c += RATE;
  }
  from_bit_interleaving(tmp0, x0);
  from_bit_interleaving(tmp1, x1);
  for (i = 0; i < clen; ++i, ++m, ++c) {
    if (i < 8) {
      *m = EXT_BYTE64(tmp0, i) ^ *c;
      tmp0 &= ~INS_BYTE64(0xff, i);
      tmp0 |= INS_BYTE64(*c, i);
    } else {
      *m = EXT_BYTE64(tmp1, i % 8) ^ *c;
      tmp1 &= ~INS_BYTE64(0xff, i % 8);
      tmp1 |= INS_BYTE64(*c, i % 8);
    }
  }
  if (clen < 8)
    tmp0 ^= INS_BYTE64(0x80, clen);
  else
    tmp1 ^= INS_BYTE64(0x80, clen % 8);
  to_bit_interleaving(x0, tmp0);
  to_bit_interleaving(x1, tmp1);

  // finalization
  x2.e ^= K0.e;
  x2.o ^= K0.o;
  x3.e ^= K1.e;
  x3.o ^= K1.o;
  P12();
  x3.e ^= K0.e;
  x3.o ^= K0.o;
  x4.e ^= K1.e;
  x4.o ^= K1.o;

  // verify tag (should be constant time, check compiler output)
  to_bit_interleaving(t0, U64BIG(*(u64*)c));
  to_bit_interleaving(t1, U64BIG(*(u64*)(c + 8)));
  if (((x3.e ^ t0.e) | (x3.o ^ t0.o) | (x4.e ^ t1.e) | (x4.o ^ t1.o)) != 0) {
    *mlen = 0;
    return -1;
  }

  return 0;
}

