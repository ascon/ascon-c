#ifndef ENDIAN_H_
#define ENDIAN_H_

typedef unsigned char u8;
typedef unsigned long long u64;

/* define default Ascon data access */
#if !defined(ASCON_UNALIGNED) && !defined(ASCON_MEMCPY) && \
    !defined(ASCON_BYTEWISE)
#define ASCON_MEMCPY
#endif

#define EXT_BYTE64(x, n) ((u8)((u64)(x) >> (8 * (7 - (n)))))
#define INS_BYTE64(x, n) ((u64)(x) << (8 * (7 - (n))))

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

/* macros for big endian machines */
#ifndef NDEBUG
#pragma message("Using macros for big endian machines")
#endif
#define U64BIG(x) (x)
#define U32BIG(x) (x)
#define U16BIG(x) (x)

#elif defined(_MSC_VER) || \
    (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

/* macros for little endian machines */
#ifndef NDEBUG
#pragma message("Using macros for little endian machines")
#endif
#define U64BIG(x)                                                              \
  ((((x)&0x00000000000000FFULL) << 56) | (((x)&0x000000000000FF00ULL) << 40) | \
   (((x)&0x0000000000FF0000ULL) << 24) | (((x)&0x00000000FF000000ULL) << 8) |  \
   (((x)&0x000000FF00000000ULL) >> 8) | (((x)&0x0000FF0000000000ULL) >> 24) |  \
   (((x)&0x00FF000000000000ULL) >> 40) | (((x)&0xFF00000000000000ULL) >> 56))
#define U32BIG(x)                                       \
  ((((x)&0x000000FF) << 24) | (((x)&0x0000FF00) << 8) | \
   (((x)&0x00FF0000) >> 8) | (((x)&0xFF000000) >> 24))
#define U16BIG(x) ((((x)&0x00FF) << 8) | (((x)&0xFF00) >> 8))

#else
#error "Ascon byte order macros not defined in endian.h"
#endif

#if defined(ASCON_UNALIGNED)

#ifndef NDEBUG
#pragma message("Using unaligned data access")
#endif

#define LOAD64(x) (U64BIG(*(u64*)(x)))
#define STORE64(x, y) (*(u64*)(x) = U64BIG(y))

#elif defined(ASCON_MEMCPY)

#ifndef NDEBUG
#pragma message("Using memcpy to access data")
#endif

#include <string.h>

static inline u64 LOAD64(const u8* bytes) {
  u64 ret_val;
  memcpy((u8*)&ret_val, bytes, 8);
  return U64BIG(ret_val);
}

static inline void STORE64(u8* bytes, u64 y) {
  y = U64BIG(y);
  memcpy(bytes, (u8*)&y, 8);
}

#elif defined(ASCON_BYTEWISE)

#ifndef NDEBUG
#pragma message("Using bytewise data access")
#endif

static inline u64 LOAD64(const u8* bytes) {
  u64 ret_val = 0;
  u8 i;
  for (i = 0; i < 8; i++) {
    ret_val |= (((u64)*bytes) << ((7 - i) << 3));
    bytes++;
  }
  return ret_val;
}

static inline void STORE64(u8* bytes, u64 y) {
  u8 i;
  for (i = 0; i < 8; i++) {
    *bytes = (y) >> ((7 - i) << 3);
    bytes++;
  }
}

#else
#error "Ascon data access macros not defined in endian.h"
#endif

#endif /* ENDIAN_H_ */
