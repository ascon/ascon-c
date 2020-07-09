#include "core.h"

void process_data(state* s, unsigned char* out, const unsigned char* in,
                  unsigned long long len, u8 mode) {
  u32_2 t0;
  u64 tmp0;
  u64 i;

  while (len >= RATE) {
    tmp0 = LOAD64(in);
    t0 = to_bit_interleaving(tmp0);
    s->x0.e ^= t0.e;
    s->x0.o ^= t0.o;
    if (mode != ASCON_AD) {
      tmp0 = from_bit_interleaving(s->x0);
      STORE64(out, tmp0);
    }
    if (mode == ASCON_DEC) s->x0 = t0;
    P(s, PB_ROUNDS);
    in += RATE;
    out += RATE;
    len -= RATE;
  }

  tmp0 = 0;
  for (i = 0; i < len; ++i, ++in) tmp0 |= INS_BYTE64(*in, i);
  in -= len;
  tmp0 |= INS_BYTE64(0x80, len);
  t0 = to_bit_interleaving(tmp0);
  s->x0.e ^= t0.e;
  s->x0.o ^= t0.o;
  if (mode != ASCON_AD) {
    tmp0 = from_bit_interleaving(s->x0);
    for (i = 0; i < len; ++i, ++out) *out = EXT_BYTE64(tmp0, i);
  }
  if (mode == ASCON_DEC) {
    for (i = 0; i < len; ++i, ++in) {
      tmp0 &= ~INS_BYTE64(0xff, i);
      tmp0 |= INS_BYTE64(*in, i);
    }
    s->x0 = to_bit_interleaving(tmp0);
  }
}

void ascon_core(state* s, unsigned char* out, const unsigned char* in,
                unsigned long long tlen, const unsigned char* ad,
                unsigned long long adlen, const unsigned char* npub,
                const unsigned char* k, u8 mode) {
  u32_2 K0, K1, N0, N1;

  /* load key and nonce */
  K0 = to_bit_interleaving(LOAD64(k));
  K1 = to_bit_interleaving(LOAD64((k + 8)));
  N0 = to_bit_interleaving(LOAD64(npub));
  N1 = to_bit_interleaving(LOAD64((npub + 8)));

  /* initialization */
  s->x0 = to_bit_interleaving(IV);
  s->x1.o = K0.o;
  s->x1.e = K0.e;
  s->x2.e = K1.e;
  s->x2.o = K1.o;
  s->x3.e = N0.e;
  s->x3.o = N0.o;
  s->x4.e = N1.e;
  s->x4.o = N1.o;
  P(s, PA_ROUNDS);
  s->x3.e ^= K0.e;
  s->x3.o ^= K0.o;
  s->x4.e ^= K1.e;
  s->x4.o ^= K1.o;

  /* process associated data */
  if (adlen) {
    process_data(s, (void*)0, ad, adlen, ASCON_AD);
    P(s, PB_ROUNDS);
  }
  s->x4.e ^= 1;

  /* process plaintext/ciphertext */
  process_data(s, out, in, tlen, mode);

  /* finalization */
  s->x1.e ^= K0.e;
  s->x1.o ^= K0.o;
  s->x2.e ^= K1.e;
  s->x2.o ^= K1.o;
  P(s, PA_ROUNDS);
  s->x3.e ^= K0.e;
  s->x3.o ^= K0.o;
  s->x4.e ^= K1.e;
  s->x4.o ^= K1.o;
}
