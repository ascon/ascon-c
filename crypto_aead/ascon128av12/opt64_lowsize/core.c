#include "core.h"

void process_data(state* s, unsigned char* out, const unsigned char* in,
                  unsigned long long len, u8 mode) {
  u64* x;
  u64 i;

  while (len >= RATE) {
    s->x0 ^= LOAD64(in);
    s->x1 ^= LOAD64(in + 8);
    if (mode != ASCON_AD) {
      STORE64(out, s->x0);
      STORE64((out + 8), s->x1);
    }
    if (mode == ASCON_DEC) {
      s->x0 = LOAD64(in);
      s->x1 = LOAD64(in + 8);
    }
    P(s, PB_ROUNDS);
    in += RATE;
    out += RATE;
    len -= RATE;
  }

  for (i = 0; i < len; ++i, ++out, ++in) {
    if (i < 8)
      x = &(s->x0);
    else
      x = &(s->x1);
    *x ^= INS_BYTE64(*in, i % 8);
    if (mode != ASCON_AD) *out = EXT_BYTE64(*x, i % 8);
    if (mode == ASCON_DEC) {
      *x &= ~INS_BYTE64(0xff, i % 8);
      *x |= INS_BYTE64(*in, i % 8);
    }
  }
  if (len < 8)
    s->x0 ^= INS_BYTE64(0x80, len);
  else
    s->x1 ^= INS_BYTE64(0x80, len % 8);
}

void ascon_core(state* s, unsigned char* out, const unsigned char* in,
                unsigned long long tlen, const unsigned char* ad,
                unsigned long long adlen, const unsigned char* npub,
                const unsigned char* k, u8 mode) {
  const u64 K0 = LOAD64(k);
  const u64 K1 = LOAD64(k + 8);
  const u64 N0 = LOAD64(npub);
  const u64 N1 = LOAD64(npub + 8);

  /* initialization */
  s->x0 = IV;
  s->x1 = K0;
  s->x2 = K1;
  s->x3 = N0;
  s->x4 = N1;
  P(s, PA_ROUNDS);
  s->x3 ^= K0;
  s->x4 ^= K1;

  /* process associated data */
  if (adlen) {
    process_data(s, (void*)0, ad, adlen, ASCON_AD);
    P(s, PB_ROUNDS);
  }
  s->x4 ^= 1;

  /* process plaintext/ciphertext */
  process_data(s, out, in, tlen, mode);

  /* finalization */
  s->x2 ^= K0;
  s->x3 ^= K1;
  P(s, PA_ROUNDS);
  s->x3 ^= K0;
  s->x4 ^= K1;
}
