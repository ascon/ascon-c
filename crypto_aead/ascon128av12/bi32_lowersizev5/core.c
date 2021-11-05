#include <string.h>
#include "core.h"

void process_data(state* s, unsigned char* out, const unsigned char* in,
                  unsigned long long len, u8 mode) {
  u32_2 t0, t1;
  u64 tmp0, tmp1;

  while (len >= RATE) {
    tmp0 = (*(u64*)in);
    t0 = to_bit_interleaving_big(tmp0);
    s->x0.e ^= t0.e;
    s->x0.o ^= t0.o;
    tmp1 = (*(u64*)(in + 8));
    t1 = to_bit_interleaving_big(tmp1);
    s->x1.e ^= t1.e;
    s->x1.o ^= t1.o;
    if (mode != ASCON_AD) {
      tmp0 = from_bit_interleaving_big(s->x0);
      *(u64*)out = (tmp0);
      tmp1 = from_bit_interleaving_big(s->x1);
      *(u64*)(out + 8) = (tmp1);
    }
    if (mode == ASCON_DEC) {
      s->x0 = t0;
      s->x1 = t1;
    }
    P(s, PB_ROUNDS);
    in += RATE;
    out += RATE;
    len -= RATE;
  }

  u8 bytes[16];
  memcpy(bytes, in, len);
  memset(bytes + len, 0, 16 - len);
  bytes[len] ^= 0x80;

  u64* tmp = (u64*)bytes;

  t0 = to_bit_interleaving_big(tmp[0]);
  s->x0.e ^= t0.e;
  s->x0.o ^= t0.o;
  t1 = to_bit_interleaving_big(tmp[1]);
  s->x1.e ^= t1.e;
  s->x1.o ^= t1.o;

  if (mode != ASCON_AD) {
    tmp[0] = from_bit_interleaving_big(s->x0);
    tmp[1] = from_bit_interleaving_big(s->x1);
    memcpy(out, bytes, len);
  }

  if (mode == ASCON_DEC) {
    memcpy(bytes, in, len);
    s->x0 = to_bit_interleaving_big(tmp[0]);
    s->x1 = to_bit_interleaving_big(tmp[1]);
  }
}

void ascon_core(state* s, unsigned char* out, const unsigned char* in,
                unsigned long long tlen, const unsigned char* ad,
                unsigned long long adlen, const unsigned char* npub,
                const unsigned char* k, u8 mode) {
  u32_2 K0, K1, N0, N1;

  // load key and nonce
  K0 = to_bit_interleaving_big((*(u64*)k));
  K1 = to_bit_interleaving_big((*(u64*)(k + 8)));
  N0 = to_bit_interleaving_big((*(u64*)npub));
  N1 = to_bit_interleaving_big((*(u64*)(npub + 8)));

  // initialization
  to_bit_interleaving_big_immediate(s->x0, IV);
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

  // process associated data
  if (adlen) {
    process_data(s, (void*)0, ad, adlen, ASCON_AD);
    P(s, PB_ROUNDS);
  }
  s->x4.e ^= 1;

  // process plaintext/ciphertext
  process_data(s, out, in, tlen, mode);

  // finalization
  s->x2.e ^= K0.e;
  s->x2.o ^= K0.o;
  s->x3.e ^= K1.e;
  s->x3.o ^= K1.o;
  P(s, PA_ROUNDS);
  s->x3.e ^= K0.e;
  s->x3.o ^= K0.o;
  s->x4.e ^= K1.e;
  s->x4.o ^= K1.o;
}
