#include "core.h"

#include <string.h>

#include "constants.h"
#include "printstate.h"

#define PA_START_ROUND 0xf0

#if ASCON_AEAD_RATE == 8
#define PB_START_ROUND 0x96
#else
#define PB_START_ROUND 0xb4
#endif

void ascon_duplex(ascon_state_t* s, unsigned char* out, const unsigned char* in,
                  unsigned long len, uint8_t mode) {
#if ASCON_AEAD_RATE == 8
  uint32x2_t tmp[1], tmp0[1];
#else
  uint32x2_t tmp[2], tmp0[2];
#endif

  while (len >= ASCON_AEAD_RATE) {
    tmp[0].l = ((uint32_t*)in)[0];
    tmp[0].h = ((uint32_t*)in)[1];
    tmp[0].x = U64LE(tmp[0].x);
    s->w[0].h ^= tmp[0].h;
    s->w[0].l ^= tmp[0].l;
#if ASCON_AEAD_RATE == 16
    tmp[1].l = ((uint32_t*)in)[2];
    tmp[1].h = ((uint32_t*)in)[3];
    tmp[1].x = U64LE(tmp[1].x);
    s->w[1].h ^= tmp[1].h;
    s->w[1].l ^= tmp[1].l;
#endif

    if (mode != ASCON_AD) {
      tmp0[0] = s->w[0];
      tmp0[0].x = U64LE(tmp0[0].x);
      ((uint32_t*)out)[0] = tmp0[0].l;
      ((uint32_t*)out)[1] = tmp0[0].h;
#if ASCON_AEAD_RATE == 16
      tmp0[1] = s->w[1];
      tmp0[1].x = U64LE(tmp0[1].x);
      ((uint32_t*)out)[2] = tmp0[1].l;
      ((uint32_t*)out)[3] = tmp0[1].h;
#endif
    }
    if (mode == ASCON_DEC) {
      s->w[0] = tmp[0];
#if ASCON_AEAD_RATE == 16
      s->w[1] = tmp[1];
#endif
    }
    if (mode == ASCON_AD) printstate("absorb adata", s);
    if (mode == ASCON_ENC) printstate("absorb plaintext", s);
    if (mode == ASCON_DEC) printstate("insert ciphertext", s);

    P(s, PB_START_ROUND);

    in += ASCON_AEAD_RATE;
    out += ASCON_AEAD_RATE;
    len -= ASCON_AEAD_RATE;
  }

  uint8_t* bytes = (uint8_t*)&tmp;
  memset(bytes, 0, sizeof tmp);
  memcpy(bytes, in, len);
  bytes[len] ^= 0x01;

  tmp[0].x = U64LE(tmp[0].x);
  s->w[0].h ^= tmp[0].h;
  s->w[0].l ^= tmp[0].l;
#if ASCON_AEAD_RATE == 16
  tmp[1].x = U64LE(tmp[1].x);
  s->w[1].h ^= tmp[1].h;
  s->w[1].l ^= tmp[1].l;
#endif

  if (mode != ASCON_AD) {
    tmp[0] = s->w[0];
    tmp[0].x = U64LE(tmp[0].x);
#if ASCON_AEAD_RATE == 16
    tmp[1] = s->w[1];
    tmp[1].x = U64LE(tmp[1].x);
#endif
    memcpy(out, bytes, len);
  }
  if (mode == ASCON_DEC) {
    memcpy(bytes, in, len);
    tmp[0].x = U64LE(tmp[0].x);
    s->w[0] = tmp[0];
#if ASCON_AEAD_RATE == 16
    tmp[1].x = U64LE(tmp[1].x);
    s->w[1] = tmp[1];
#endif
  }
  if (mode == ASCON_AD) printstate("pad adata", s);
  if (mode == ASCON_ENC) printstate("pad plaintext", s);
  if (mode == ASCON_DEC) printstate("pad ciphertext", s);
}

void ascon_core(ascon_state_t* s, unsigned char* out, const unsigned char* in,
                unsigned long long tlen, const unsigned char* ad,
                unsigned long long adlen, const unsigned char* npub,
                const unsigned char* k, uint8_t mode) {
  uint32x2_t tmp[2];
  uint32x2_t K0, K1, N0, N1;

  // load key
#if CRYPTO_KEYBYTES == 20
  uint32x2_t K2;
#if (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
  K0.l = U32LE(((uint32_t*)k)[0]);
#else
  K0.h = U32LE(((uint32_t*)k)[0]);
#endif
  tmp[0].l = ((uint32_t*)k)[1];
  tmp[0].h = ((uint32_t*)k)[2];
  tmp[1].l = ((uint32_t*)k)[3];
  tmp[1].h = ((uint32_t*)k)[4];
  tmp[0].x = U64LE(tmp[0].x);
  tmp[1].x = U64LE(tmp[1].x);
  K1 = tmp[0];
  K2 = tmp[1];
#else
  tmp[0].l = ((uint32_t*)k)[0];
  tmp[0].h = ((uint32_t*)k)[1];
  tmp[1].l = ((uint32_t*)k)[2];
  tmp[1].h = ((uint32_t*)k)[3];
  tmp[0].x = U64LE(tmp[0].x);
  tmp[1].x = U64LE(tmp[1].x);
  K0 = tmp[0];
  K1 = tmp[1];
#endif

  // load nonce
  tmp[0].l = ((uint32_t*)npub)[0];
  tmp[0].h = ((uint32_t*)npub)[1];
  tmp[1].l = ((uint32_t*)npub)[2];
  tmp[1].h = ((uint32_t*)npub)[3];
  tmp[0].x = U64LE(tmp[0].x);
  tmp[1].x = U64LE(tmp[1].x);
  N0 = tmp[0];
  N1 = tmp[1];

  // initialization
#if CRYPTO_KEYBYTES == 20
#if (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
  s->w[0].h = (uint32_t)ASCON_80PQ_IV;
  s->w[0].l = K0.l;
#else
  s->w[0].l = (uint32_t)ASCON_80PQ_IV;
  s->w[0].h = K0.h;
#endif
  s->w[1].h = K1.h;
  s->w[1].l = K1.l;
  s->w[2].h = K2.h;
  s->w[2].l = K2.l;
#else
#if ASCON_AEAD_RATE == 8
  s->x[0] = ASCON_128_IV;
#else
  s->x[0] = ASCON_128A_IV;
#endif
  s->w[1].h = K0.h;
  s->w[1].l = K0.l;
  s->w[2].h = K1.h;
  s->w[2].l = K1.l;
#endif
  s->w[3].h = N0.h;
  s->w[3].l = N0.l;
  s->w[4].h = N1.h;
  s->w[4].l = N1.l;
  printstate("init 1st key xor", s);
  P(s, PA_START_ROUND);
#if CRYPTO_KEYBYTES == 20
#if (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
  s->w[2].l ^= K0.l;
#else
  s->w[2].h ^= K0.h;
#endif
  s->w[3].h ^= K1.h;
  s->w[3].l ^= K1.l;
  s->w[4].h ^= K2.h;
  s->w[4].l ^= K2.l;
#else
  s->w[3].h ^= K0.h;
  s->w[3].l ^= K0.l;
  s->w[4].h ^= K1.h;
  s->w[4].l ^= K1.l;
#endif
  printstate("init 2nd key xor", s);

  // process associated data
  if (adlen) {
    ascon_duplex(s, (void*)0, ad, adlen, ASCON_AD);
    P(s, PB_START_ROUND);
  }
  s->x[4] ^= 0x8000000000000000;
  printstate("domain separation", s);

  // process plaintext/ciphertext
  ascon_duplex(s, out, in, tlen, mode);

  // finalization
#if CRYPTO_KEYBYTES == 20
#if (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
  s->w[1].h ^= K0.l;
  s->w[1].l ^= K1.h;
  s->w[2].h ^= K1.l;
  s->w[2].l ^= K2.h;
  s->w[3].h ^= K2.l;
#else
  s->w[1].l ^= K0.h;
  s->w[1].h ^= K1.l;
  s->w[2].l ^= K1.h;
  s->w[2].h ^= K2.l;
  s->w[3].l ^= K2.h;
#endif
#elif ASCON_AEAD_RATE == 8
  s->w[1].h ^= K0.h;
  s->w[1].l ^= K0.l;
  s->w[2].h ^= K1.h;
  s->w[2].l ^= K1.l;
#else
  s->w[2].h ^= K0.h;
  s->w[2].l ^= K0.l;
  s->w[3].h ^= K1.h;
  s->w[3].l ^= K1.l;
#endif
  printstate("final 1st key xor", s);
  P(s, PA_START_ROUND);
#if CRYPTO_KEYBYTES == 20
  s->w[3].h ^= K1.h;
  s->w[3].l ^= K1.l;
  s->w[4].h ^= K2.h;
  s->w[4].l ^= K2.l;
#else
  s->w[3].h ^= K0.h;
  s->w[3].l ^= K0.l;
  s->w[4].h ^= K1.h;
  s->w[4].l ^= K1.l;
#endif
  s->x[3] = U64LE(s->x[3]);
  s->x[4] = U64LE(s->x[4]);
  printstate("final 2nd key xor", s);
}
