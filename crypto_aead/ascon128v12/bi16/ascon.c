#include "api.h"
#include "endian.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#define RATE (64 / 8)
#define PA_ROUNDS 12
#define PB_ROUNDS 6

#define EXT_BYTE16(x, n) ((u8)((u16)(x) >> (8 * (1 - (n)))))
#define INS_BYTE16(x, n) ((u16)(x) << (8 * (1 - (n))))
#define ROTR16(x, n) (((x) >> (n)) | ((x) << (16 - (n))))
#define ROTL16(x, n) (((x) << (n)) | ((x) >> (16 - (n))))

#define COMPRESS_LONG_16(x)      \
  do {                           \
    x &= 0x1111;                 \
    x = (x | (x >> 3)) & 0x0303; \
    x = (x | (x >> 6)) & 0x000f; \
  } while (0)

#define COMPRESS_U16(var, var_3, var_2, var_1, var_0) \
  do {                                                \
    /* var 16-bit, and var_0/1/2/3 4-bit */           \
    var_0 = var;                                      \
    var_1 = var_0 >> 1;                               \
    var_2 = var_1 >> 1;                               \
    var_3 = var_2 >> 1;                               \
    COMPRESS_LONG_16(var_0);                          \
    COMPRESS_LONG_16(var_1);                          \
    COMPRESS_LONG_16(var_2);                          \
    COMPRESS_LONG_16(var_3);                          \
  } while (0)

#define COMPRESS_BYTE_ARRAY_16(a, var_3, var_2, var_1, var_0)          \
  do {                                                                 \
    COMPRESS_U16(U16BIG(((u16 *)(a))[3]), var_3, var_2, var_1, var_0); \
    COMPRESS_U16(U16BIG(((u16 *)(a))[2]), t1_3, t1_2, t1_1, t1_0);     \
    var_0 |= t1_0 << 4;                                                \
    var_1 |= t1_1 << 4;                                                \
    var_2 |= t1_2 << 4;                                                \
    var_3 |= t1_3 << 4;                                                \
    COMPRESS_U16(U16BIG(((u16 *)(a))[1]), t1_3, t1_2, t1_1, t1_0);     \
    var_0 |= t1_0 << 8;                                                \
    var_1 |= t1_1 << 8;                                                \
    var_2 |= t1_2 << 8;                                                \
    var_3 |= t1_3 << 8;                                                \
    COMPRESS_U16(U16BIG(((u16 *)(a))[0]), t1_3, t1_2, t1_1, t1_0);     \
    var_0 |= t1_0 << 12;                                               \
    var_1 |= t1_1 << 12;                                               \
    var_2 |= t1_2 << 12;                                               \
    var_3 |= t1_3 << 12;                                               \
  } while (0)

#define EXPAND_SHORT_16(x)       \
  do {                           \
    x &= 0x000f;                 \
    x = (x | (x << 6)) & 0x0303; \
    x = (x | (x << 3)) & 0x1111; \
  } while (0)

#define EXPAND_U16(var, var_3, var_2, var_1, var_0)                \
  do {                                                             \
    /* var 16-bit, and var_0/1/2/3 4-bit */                        \
    t0_0 = var_0;                                                  \
    t0_1 = var_1;                                                  \
    t0_2 = var_2;                                                  \
    t0_3 = var_3;                                                  \
    EXPAND_SHORT_16(t0_0);                                         \
    EXPAND_SHORT_16(t0_1);                                         \
    EXPAND_SHORT_16(t0_2);                                         \
    EXPAND_SHORT_16(t0_3);                                         \
    *var = U16BIG(t0_0 | (t0_1 << 1) | (t0_2 << 2) | (t0_3 << 3)); \
  } while (0)

#define EXPAND_BYTE_ARRAY_16(a, var_3, var_2, var_1, var_0)     \
  do {                                                          \
    EXPAND_U16((((u16 *)(a)) + 3), var_3, var_2, var_1, var_0); \
    t1_3 = var_3 >> 4;                                          \
    t1_2 = var_2 >> 4;                                          \
    t1_1 = var_1 >> 4;                                          \
    t1_0 = var_0 >> 4;                                          \
    EXPAND_U16((((u16 *)(a)) + 2), t1_3, t1_2, t1_1, t1_0);     \
    t1_3 >>= 4;                                                 \
    t1_2 >>= 4;                                                 \
    t1_1 >>= 4;                                                 \
    t1_0 >>= 4;                                                 \
    EXPAND_U16((((u16 *)(a)) + 1), t1_3, t1_2, t1_1, t1_0);     \
    t1_3 >>= 4;                                                 \
    t1_2 >>= 4;                                                 \
    t1_1 >>= 4;                                                 \
    t1_0 >>= 4;                                                 \
    EXPAND_U16((((u16 *)(a)) + 0), t1_3, t1_2, t1_1, t1_0);     \
  } while (0)

// This way of implementing Ascon's S-box was inpired by personal communication
// with Joan Daemen about implementing the 3-bit chi layer.
#define ROUND_16(C_3, C_2, C_1, C_0) \
  do {                               \
    /* round constant */             \
    x2_0 ^= C_0;                     \
    x2_1 ^= C_1;                     \
    x2_2 ^= C_2;                     \
    x2_3 ^= C_3;                     \
    /* s-box layer */                \
    x0_0 ^= x4_0;                    \
    x4_0 ^= x3_0;                    \
    x2_0 ^= x1_0;                    \
    t0_0 = x0_0 & (~x4_0);           \
    t1_0 = x2_0 & (~x1_0);           \
    x0_0 ^= t1_0;                    \
    t1_0 = x4_0 & (~x3_0);           \
    x2_0 ^= t1_0;                    \
    t1_0 = x1_0 & (~x0_0);           \
    x4_0 ^= t1_0;                    \
    t1_0 = x3_0 & (~x2_0);           \
    x1_0 ^= t1_0;                    \
    x3_0 ^= t0_0;                    \
    x1_0 ^= x0_0;                    \
    x3_0 ^= x2_0;                    \
    x0_0 ^= x4_0;                    \
    x2_0 = ~x2_0;                    \
    x0_1 ^= x4_1;                    \
    x4_1 ^= x3_1;                    \
    x2_1 ^= x1_1;                    \
    t0_0 = x0_1 & (~x4_1);           \
    t1_0 = x2_1 & (~x1_1);           \
    x0_1 ^= t1_0;                    \
    t1_0 = x4_1 & (~x3_1);           \
    x2_1 ^= t1_0;                    \
    t1_0 = x1_1 & (~x0_1);           \
    x4_1 ^= t1_0;                    \
    t1_0 = x3_1 & (~x2_1);           \
    x1_1 ^= t1_0;                    \
    x3_1 ^= t0_0;                    \
    x1_1 ^= x0_1;                    \
    x3_1 ^= x2_1;                    \
    x0_1 ^= x4_1;                    \
    x2_1 = ~x2_1;                    \
    x0_2 ^= x4_2;                    \
    x4_2 ^= x3_2;                    \
    x2_2 ^= x1_2;                    \
    t0_0 = x0_2 & (~x4_2);           \
    t1_0 = x2_2 & (~x1_2);           \
    x0_2 ^= t1_0;                    \
    t1_0 = x4_2 & (~x3_2);           \
    x2_2 ^= t1_0;                    \
    t1_0 = x1_2 & (~x0_2);           \
    x4_2 ^= t1_0;                    \
    t1_0 = x3_2 & (~x2_2);           \
    x1_2 ^= t1_0;                    \
    x3_2 ^= t0_0;                    \
    x1_2 ^= x0_2;                    \
    x3_2 ^= x2_2;                    \
    x0_2 ^= x4_2;                    \
    x2_2 = ~x2_2;                    \
    x0_3 ^= x4_3;                    \
    x4_3 ^= x3_3;                    \
    x2_3 ^= x1_3;                    \
    t0_0 = x0_3 & (~x4_3);           \
    t1_0 = x2_3 & (~x1_3);           \
    x0_3 ^= t1_0;                    \
    t1_0 = x4_3 & (~x3_3);           \
    x2_3 ^= t1_0;                    \
    t1_0 = x1_3 & (~x0_3);           \
    x4_3 ^= t1_0;                    \
    t1_0 = x3_3 & (~x2_3);           \
    x1_3 ^= t1_0;                    \
    x3_3 ^= t0_0;                    \
    x1_3 ^= x0_3;                    \
    x3_3 ^= x2_3;                    \
    x0_3 ^= x4_3;                    \
    x2_3 = ~x2_3;                    \
    /* linear layer */               \
    t0_0 = x0_0;                     \
    t0_1 = x0_1;                     \
    t0_2 = x0_2;                     \
    t0_3 = x0_3;                     \
    x0_1 ^= ROTR16(t0_0, 5);         \
    x0_2 ^= ROTR16(t0_1, 5);         \
    x0_3 ^= ROTR16(t0_2, 5);         \
    x0_0 ^= ROTR16(t0_3, 4);         \
    x0_0 ^= ROTR16(t0_0, 7);         \
    x0_1 ^= ROTR16(t0_1, 7);         \
    x0_2 ^= ROTR16(t0_2, 7);         \
    x0_3 ^= ROTR16(t0_3, 7);         \
    t0_0 = x1_0;                     \
    t0_1 = x1_1;                     \
    t0_2 = x1_2;                     \
    t0_3 = x1_3;                     \
    x1_3 ^= t0_0;                    \
    x1_0 ^= ROTL16(t0_1, 1);         \
    x1_1 ^= ROTL16(t0_2, 1);         \
    x1_2 ^= ROTL16(t0_3, 1);         \
    x1_1 ^= ROTL16(t0_0, 6);         \
    x1_2 ^= ROTL16(t0_1, 6);         \
    x1_3 ^= ROTL16(t0_2, 6);         \
    x1_0 ^= ROTL16(t0_3, 7);         \
    t0_0 = x2_0;                     \
    t0_1 = x2_1;                     \
    t0_2 = x2_2;                     \
    t0_3 = x2_3;                     \
    x2_3 ^= ROTR16(t0_0, 1);         \
    x2_0 ^= t0_1;                    \
    x2_1 ^= t0_2;                    \
    x2_2 ^= t0_3;                    \
    x2_2 ^= ROTR16(t0_0, 2);         \
    x2_3 ^= ROTR16(t0_1, 2);         \
    x2_0 ^= ROTR16(t0_2, 1);         \
    x2_1 ^= ROTR16(t0_3, 1);         \
    t0_0 = x3_0;                     \
    t0_1 = x3_1;                     \
    t0_2 = x3_2;                     \
    t0_3 = x3_3;                     \
    x3_2 ^= ROTR16(t0_0, 3);         \
    x3_3 ^= ROTR16(t0_1, 3);         \
    x3_0 ^= ROTR16(t0_2, 2);         \
    x3_1 ^= ROTR16(t0_3, 2);         \
    x3_3 ^= ROTR16(t0_0, 5);         \
    x3_0 ^= ROTR16(t0_1, 4);         \
    x3_1 ^= ROTR16(t0_2, 4);         \
    x3_2 ^= ROTR16(t0_3, 4);         \
    t0_0 = x4_0;                     \
    t0_1 = x4_1;                     \
    t0_2 = x4_2;                     \
    t0_3 = x4_3;                     \
    x4_1 ^= ROTR16(t0_0, 2);         \
    x4_2 ^= ROTR16(t0_1, 2);         \
    x4_3 ^= ROTR16(t0_2, 2);         \
    x4_0 ^= ROTR16(t0_3, 1);         \
    x4_3 ^= ROTL16(t0_0, 5);         \
    x4_0 ^= ROTL16(t0_1, 6);         \
    x4_1 ^= ROTL16(t0_2, 6);         \
    x4_2 ^= ROTL16(t0_3, 6);         \
  } while (0)

#define P12_16            \
  do {                    \
    ROUND_16(2, 2, 2, 2); \
    ROUND_16(2, 2, 2, 1); \
    ROUND_16(2, 2, 1, 2); \
    ROUND_16(2, 2, 1, 1); \
    ROUND_16(2, 1, 2, 2); \
    ROUND_16(2, 1, 2, 1); \
    ROUND_16(2, 1, 1, 2); \
    ROUND_16(2, 1, 1, 1); \
    ROUND_16(1, 2, 2, 2); \
    ROUND_16(1, 2, 2, 1); \
    ROUND_16(1, 2, 1, 2); \
    ROUND_16(1, 2, 1, 1); \
  } while (0)

#define P6_16             \
  do {                    \
    ROUND_16(2, 1, 1, 2); \
    ROUND_16(2, 1, 1, 1); \
    ROUND_16(1, 2, 2, 2); \
    ROUND_16(1, 2, 2, 1); \
    ROUND_16(1, 2, 1, 2); \
    ROUND_16(1, 2, 1, 1); \
  } while (0)

int crypto_aead_encrypt(unsigned char *c, unsigned long long *clen,
                        const unsigned char *m, unsigned long long mlen,
                        const unsigned char *ad, unsigned long long adlen,
                        const unsigned char *nsec, const unsigned char *npub,
                        const unsigned char *k) {
  u64 rlen;
  u64 i;

  u8 buffer[8];

  u16 K0_0;
  u16 K1_0;
  u16 N0_0;
  u16 N1_0;
  u16 x0_0, x1_0, x2_0, x3_0, x4_0;
  u16 t0_0, t1_0;

  u16 K0_1;
  u16 K1_1;
  u16 N0_1;
  u16 N1_1;
  u16 x0_1, x1_1, x2_1, x3_1, x4_1;
  u16 t0_1, t1_1;

  u16 K0_2;
  u16 K1_2;
  u16 N0_2;
  u16 N1_2;
  u16 x0_2, x1_2, x2_2, x3_2, x4_2;
  u16 t0_2, t1_2;

  u16 K0_3;
  u16 K1_3;
  u16 N0_3;
  u16 N1_3;
  u16 x0_3, x1_3, x2_3, x3_3, x4_3;
  u16 t0_3, t1_3;

  u16 in_0, in_1, in_2, in_3;

  (void)nsec;

  COMPRESS_BYTE_ARRAY_16(k, K0_3, K0_2, K0_1, K0_0);
  COMPRESS_BYTE_ARRAY_16(k + 8, K1_3, K1_2, K1_1, K1_0);
  COMPRESS_BYTE_ARRAY_16(npub, N0_3, N0_2, N0_1, N0_0);
  COMPRESS_BYTE_ARRAY_16(npub + 8, N1_3, N1_2, N1_1, N1_0);

  // initialization
  t1_0 = (u16)((CRYPTO_KEYBYTES * 8) << 8 | (RATE * 8) << 0);
  t1_1 = t1_0 >> 1;
  t1_2 = t1_1 >> 1;
  t1_3 = t1_2 >> 1;
  COMPRESS_LONG_16(t1_0);
  COMPRESS_LONG_16(t1_1);
  COMPRESS_LONG_16(t1_2);
  COMPRESS_LONG_16(t1_3);
  x0_0 = t1_0 << 12;
  x0_1 = t1_1 << 12;
  x0_2 = t1_2 << 12;
  x0_3 = t1_3 << 12;
  t1_0 = (u16)(PA_ROUNDS << 8 | PB_ROUNDS << 0);
  t1_1 = t1_0 >> 1;
  t1_2 = t1_1 >> 1;
  t1_3 = t1_2 >> 1;
  COMPRESS_LONG_16(t1_0);
  COMPRESS_LONG_16(t1_1);
  COMPRESS_LONG_16(t1_2);
  COMPRESS_LONG_16(t1_3);
  x0_0 |= t1_0 << 8;
  x0_1 |= t1_1 << 8;
  x0_2 |= t1_2 << 8;
  x0_3 |= t1_3 << 8;
  x1_0 = K0_0;
  x1_1 = K0_1;
  x1_2 = K0_2;
  x1_3 = K0_3;
  x2_0 = K1_0;
  x2_1 = K1_1;
  x2_2 = K1_2;
  x2_3 = K1_3;
  x3_0 = N0_0;
  x3_1 = N0_1;
  x3_2 = N0_2;
  x3_3 = N0_3;
  x4_0 = N1_0;
  x4_1 = N1_1;
  x4_2 = N1_2;
  x4_3 = N1_3;
  P12_16;
  x3_0 ^= K0_0;
  x3_1 ^= K0_1;
  x3_2 ^= K0_2;
  x3_3 ^= K0_3;
  x4_0 ^= K1_0;
  x4_1 ^= K1_1;
  x4_2 ^= K1_2;
  x4_3 ^= K1_3;
  // process associated data
  if (adlen) {
    rlen = adlen;
    while (rlen >= RATE) {
      COMPRESS_BYTE_ARRAY_16(ad, in_3, in_2, in_1, in_0);
      x0_0 ^= in_0;
      x0_1 ^= in_1;
      x0_2 ^= in_2;
      x0_3 ^= in_3;
      P6_16;
      rlen -= RATE;
      ad += RATE;
    }
    for (i = 0; i < rlen; ++i, ++ad) buffer[i] = *ad;
    buffer[rlen] = 0x80;
    for (i = rlen + 1; i < 8; ++i) buffer[i] = 0;
    COMPRESS_BYTE_ARRAY_16(buffer, in_3, in_2, in_1, in_0);
    x0_0 ^= in_0;
    x0_1 ^= in_1;
    x0_2 ^= in_2;
    x0_3 ^= in_3;
    P6_16;
  }
  x4_0 ^= 1;

  // process plaintext
  rlen = mlen;
  while (rlen >= RATE) {
    COMPRESS_BYTE_ARRAY_16(m, in_3, in_2, in_1, in_0);
    x0_0 ^= in_0;
    x0_1 ^= in_1;
    x0_2 ^= in_2;
    x0_3 ^= in_3;
    EXPAND_BYTE_ARRAY_16(c, x0_3, x0_2, x0_1, x0_0);
    P6_16;
    rlen -= RATE;
    m += RATE;
    c += RATE;
  }
  for (i = 0; i < rlen; ++i, ++m) buffer[i] = *m;
  buffer[rlen] = 0x80;
  for (i = rlen + 1; i < 8; ++i) buffer[i] = 0;
  COMPRESS_BYTE_ARRAY_16(buffer, in_3, in_2, in_1, in_0);
  x0_0 ^= in_0;
  x0_1 ^= in_1;
  x0_2 ^= in_2;
  x0_3 ^= in_3;
  EXPAND_BYTE_ARRAY_16(buffer, x0_3, x0_2, x0_1, x0_0);
  for (i = 0; i < rlen; ++i, ++c) *c = buffer[i];

  // finalization
  x1_0 ^= K0_0;
  x1_1 ^= K0_1;
  x1_2 ^= K0_2;
  x1_3 ^= K0_3;
  x2_0 ^= K1_0;
  x2_1 ^= K1_1;
  x2_2 ^= K1_2;
  x2_3 ^= K1_3;
  P12_16;
  x3_0 ^= K0_0;
  x3_1 ^= K0_1;
  x3_2 ^= K0_2;
  x3_3 ^= K0_3;
  x4_0 ^= K1_0;
  x4_1 ^= K1_1;
  x4_2 ^= K1_2;
  x4_3 ^= K1_3;

  // return tag
  EXPAND_BYTE_ARRAY_16(c, x3_3, x3_2, x3_1, x3_0);
  c += 8;
  EXPAND_BYTE_ARRAY_16(c, x4_3, x4_2, x4_1, x4_0);
  *clen = mlen + CRYPTO_ABYTES;

  return 0;
}

int crypto_aead_decrypt(unsigned char *m, unsigned long long *mlen,
                        unsigned char *nsec, const unsigned char *c,
                        unsigned long long clen, const unsigned char *ad,
                        unsigned long long adlen, const unsigned char *npub,
                        const unsigned char *k) {
  *mlen = 0;
  if (clen < CRYPTO_ABYTES) return -1;

  u64 rlen;
  u64 i;

  u16 ret_val;
  u8 buffer[8];

  u16 K0_0;
  u16 K1_0;
  u16 N0_0;
  u16 N1_0;
  u16 x0_0, x1_0, x2_0, x3_0, x4_0;
  u16 t0_0, t1_0;
  u16 K0_1;
  u16 K1_1;
  u16 N0_1;
  u16 N1_1;
  u16 x0_1, x1_1, x2_1, x3_1, x4_1;
  u16 t0_1, t1_1;

  u16 K0_2;
  u16 K1_2;
  u16 N0_2;
  u16 N1_2;
  u16 x0_2, x1_2, x2_2, x3_2, x4_2;
  u16 t0_2, t1_2;

  u16 K0_3;
  u16 K1_3;
  u16 N0_3;
  u16 N1_3;
  u16 x0_3, x1_3, x2_3, x3_3, x4_3;
  u16 t0_3, t1_3;
  u16 in_0, in_1, in_2, in_3;

  (void)nsec;

  COMPRESS_BYTE_ARRAY_16(k, K0_3, K0_2, K0_1, K0_0);
  COMPRESS_BYTE_ARRAY_16(k + 8, K1_3, K1_2, K1_1, K1_0);
  COMPRESS_BYTE_ARRAY_16(npub, N0_3, N0_2, N0_1, N0_0);
  COMPRESS_BYTE_ARRAY_16(npub + 8, N1_3, N1_2, N1_1, N1_0);

  // initialization
  t1_0 = (u16)((CRYPTO_KEYBYTES * 8) << 8 | (RATE * 8) << 0);
  t1_1 = t1_0 >> 1;
  t1_2 = t1_1 >> 1;
  t1_3 = t1_2 >> 1;
  COMPRESS_LONG_16(t1_0);
  COMPRESS_LONG_16(t1_1);
  COMPRESS_LONG_16(t1_2);
  COMPRESS_LONG_16(t1_3);
  x0_0 = t1_0 << 12;
  x0_1 = t1_1 << 12;
  x0_2 = t1_2 << 12;
  x0_3 = t1_3 << 12;
  t1_0 = (u16)(PA_ROUNDS << 8 | PB_ROUNDS << 0);
  t1_1 = t1_0 >> 1;
  t1_2 = t1_1 >> 1;
  t1_3 = t1_2 >> 1;
  COMPRESS_LONG_16(t1_0);
  COMPRESS_LONG_16(t1_1);
  COMPRESS_LONG_16(t1_2);
  COMPRESS_LONG_16(t1_3);
  x0_0 |= t1_0 << 8;
  x0_1 |= t1_1 << 8;
  x0_2 |= t1_2 << 8;
  x0_3 |= t1_3 << 8;
  x1_0 = K0_0;
  x1_1 = K0_1;
  x1_2 = K0_2;
  x1_3 = K0_3;
  x2_0 = K1_0;
  x2_1 = K1_1;
  x2_2 = K1_2;
  x2_3 = K1_3;
  x3_0 = N0_0;
  x3_1 = N0_1;
  x3_2 = N0_2;
  x3_3 = N0_3;
  x4_0 = N1_0;
  x4_1 = N1_1;
  x4_2 = N1_2;
  x4_3 = N1_3;
  P12_16;
  x3_0 ^= K0_0;
  x3_1 ^= K0_1;
  x3_2 ^= K0_2;
  x3_3 ^= K0_3;
  x4_0 ^= K1_0;
  x4_1 ^= K1_1;
  x4_2 ^= K1_2;
  x4_3 ^= K1_3;
  // process associated data
  if (adlen) {
    rlen = adlen;
    while (rlen >= RATE) {
      COMPRESS_BYTE_ARRAY_16(ad, in_3, in_2, in_1, in_0);
      x0_0 ^= in_0;
      x0_1 ^= in_1;
      x0_2 ^= in_2;
      x0_3 ^= in_3;
      P6_16;
      rlen -= RATE;
      ad += RATE;
    }
    for (i = 0; i < rlen; ++i, ++ad) buffer[i] = *ad;
    buffer[rlen] = 0x80;
    for (i = rlen + 1; i < 8; ++i) buffer[i] = 0;
    COMPRESS_BYTE_ARRAY_16(buffer, in_3, in_2, in_1, in_0);
    x0_0 ^= in_0;
    x0_1 ^= in_1;
    x0_2 ^= in_2;
    x0_3 ^= in_3;
    P6_16;
  }
  x4_0 ^= 1;

  // process plaintext
  rlen = clen - CRYPTO_KEYBYTES;
  while (rlen >= RATE) {
    EXPAND_U16(&t1_0, x0_3, x0_2, x0_1, x0_0);
    EXPAND_U16(&t1_1, x0_3 >> 4, x0_2 >> 4, x0_1 >> 4, x0_0 >> 4);
    EXPAND_U16(&t1_2, x0_3 >> 8, x0_2 >> 8, x0_1 >> 8, x0_0 >> 8);
    EXPAND_U16(&t1_3, x0_3 >> 12, x0_2 >> 12, x0_1 >> 12, x0_0 >> 12);
    ((u16 *)m)[0] = (t1_3) ^ ((u16 *)c)[0];
    ((u16 *)m)[1] = (t1_2) ^ ((u16 *)c)[1];
    ((u16 *)m)[2] = (t1_1) ^ ((u16 *)c)[2];
    ((u16 *)m)[3] = (t1_0) ^ ((u16 *)c)[3];
    COMPRESS_BYTE_ARRAY_16(c, x0_3, x0_2, x0_1, x0_0);
    P6_16;
    rlen -= RATE;
    m += RATE;
    c += RATE;
  }
  EXPAND_BYTE_ARRAY_16(buffer, x0_3, x0_2, x0_1, x0_0);
  for (i = 0; i < rlen; ++i, ++m, ++c) {
    *m = buffer[i] ^ *c;
    buffer[i] = *c;
  }
  buffer[rlen] ^= 0x80;

  COMPRESS_BYTE_ARRAY_16(buffer, x0_3, x0_2, x0_1, x0_0);

  // finalization
  x1_0 ^= K0_0;
  x1_1 ^= K0_1;
  x1_2 ^= K0_2;
  x1_3 ^= K0_3;
  x2_0 ^= K1_0;
  x2_1 ^= K1_1;
  x2_2 ^= K1_2;
  x2_3 ^= K1_3;
  P12_16;
  x3_0 ^= K0_0;
  x3_1 ^= K0_1;
  x3_2 ^= K0_2;
  x3_3 ^= K0_3;
  x4_0 ^= K1_0;
  x4_1 ^= K1_1;
  x4_2 ^= K1_2;
  x4_3 ^= K1_3;

  // return -1 if verification fails
  ret_val = 0;
  EXPAND_U16(&t1_0, x3_3, x3_2, x3_1, x3_0);
  EXPAND_U16(&t1_1, x3_3 >> 4, x3_2 >> 4, x3_1 >> 4, x3_0 >> 4);
  EXPAND_U16(&t1_2, x3_3 >> 8, x3_2 >> 8, x3_1 >> 8, x3_0 >> 8);
  EXPAND_U16(&t1_3, x3_3 >> 12, x3_2 >> 12, x3_1 >> 12, x3_0 >> 12);

  ret_val |= ((u16 *)c)[0] ^ (t1_3);
  ret_val |= ((u16 *)c)[1] ^ (t1_2);
  ret_val |= ((u16 *)c)[2] ^ (t1_1);
  ret_val |= ((u16 *)c)[3] ^ (t1_0);

  EXPAND_U16(&t1_0, x4_3, x4_2, x4_1, x4_0);
  EXPAND_U16(&t1_1, x4_3 >> 4, x4_2 >> 4, x4_1 >> 4, x4_0 >> 4);
  EXPAND_U16(&t1_2, x4_3 >> 8, x4_2 >> 8, x4_1 >> 8, x4_0 >> 8);
  EXPAND_U16(&t1_3, x4_3 >> 12, x4_2 >> 12, x4_1 >> 12, x4_0 >> 12);

  ret_val |= ((u16 *)c)[4] ^ (t1_3);
  ret_val |= ((u16 *)c)[5] ^ (t1_2);
  ret_val |= ((u16 *)c)[6] ^ (t1_1);
  ret_val |= ((u16 *)c)[7] ^ (t1_0);

  if (ret_val != 0) return -1;

  // return plaintext
  *mlen = clen - CRYPTO_ABYTES;
  return 0;
}

