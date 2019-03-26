#include "api.h"
#include "endian.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#define RATE (64 / 8)
#define PA_ROUNDS 12

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

int crypto_hash(unsigned char *out, const unsigned char *in,
                unsigned long long inlen) {
  u64 rlen;
  u64 i;

  u8 buffer[8];

  u16 x0_0, x1_0, x2_0, x3_0, x4_0;
  u16 t0_0, t1_0;

  u16 x0_1, x1_1, x2_1, x3_1, x4_1;
  u16 t0_1, t1_1;

  u16 x0_2, x1_2, x2_2, x3_2, x4_2;
  u16 t0_2, t1_2;

  u16 x0_3, x1_3, x2_3, x3_3, x4_3;
  u16 t0_3, t1_3;

  u16 in_0, in_1, in_2, in_3;

  // initialization
  x0_0 = 0x38db;
  x0_1 = 0xd37a;
  x0_2 = 0xc0b9;
  x0_3 = 0xefc9;
  x1_0 = 0x6b18;
  x1_1 = 0x7251;
  x1_2 = 0xd0;
  x1_3 = 0xe490;
  x2_0 = 0x8bb8;
  x2_1 = 0x9507;
  x2_2 = 0x423a;
  x2_3 = 0xbbec;
  x3_0 = 0x6da4;
  x3_1 = 0x42ae;
  x3_2 = 0x802a;
  x3_3 = 0x1cfb;
  x4_0 = 0x95d4;
  x4_1 = 0x9828;
  x4_2 = 0x56da;
  x4_3 = 0x3b88;

  // absorb
  rlen = inlen;
  while (rlen >= RATE) {
    COMPRESS_BYTE_ARRAY_16(in, in_3, in_2, in_1, in_0);
    x0_0 ^= in_0;
    x0_1 ^= in_1;
    x0_2 ^= in_2;
    x0_3 ^= in_3;
    P12_16;
    rlen -= RATE;
    in += RATE;
  }
  for (i = 0; i < rlen; ++i, ++in) buffer[i] = *in;
  buffer[rlen] = 0x80;
  for (i = rlen + 1; i < 8; ++i) buffer[i] = 0;
  COMPRESS_BYTE_ARRAY_16(buffer, in_3, in_2, in_1, in_0);
  x0_0 ^= in_0;
  x0_1 ^= in_1;
  x0_2 ^= in_2;
  x0_3 ^= in_3;
  P12_16;

  // squeeze (only full blocks)
  rlen = CRYPTO_BYTES;
  while (rlen > RATE) {
    EXPAND_BYTE_ARRAY_16(out, x0_3, x0_2, x0_1, x0_0);
    P12_16;
    rlen -= RATE;
    out += RATE;
  }
  EXPAND_BYTE_ARRAY_16(out, x0_3, x0_2, x0_1, x0_0);

  return 0;
}

