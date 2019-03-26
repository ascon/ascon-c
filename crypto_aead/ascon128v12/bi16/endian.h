#ifndef ENDIAN_H_
#define ENDIAN_H_

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

// macros for big endian machines
#define U64BIG(x) (x)
#define U32BIG(x) (x)
#define U16BIG(x) (x)

#elif defined(_MSC_VER) || \
  (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

// macros for little endian machines
#define U64BIG(x)                                                                  \
  ((((x) & 0x00000000000000FFULL) << 56) | (((x) & 0x000000000000FF00ULL) << 40) | \
   (((x) & 0x0000000000FF0000ULL) << 24) | (((x) & 0x00000000FF000000ULL) << 8) |  \
   (((x) & 0x000000FF00000000ULL) >> 8) | (((x) & 0x0000FF0000000000ULL) >> 24) |  \
   (((x) & 0x00FF000000000000ULL) >> 40) | (((x) & 0xFF00000000000000ULL) >> 56))
#define U32BIG(x)                                           \
  ((((x) & 0x000000FF) << 24) | (((x) & 0x0000FF00) << 8) | \
   (((x) & 0x00FF0000) >> 8) | (((x) & 0xFF000000) >> 24))
#define U16BIG(x)                                 \
  ((((x) & 0x00FF) << 8) | (((x) & 0xFF00) >> 8))

#else
#error "ascon byte order macros not defined in endian.h"
#endif

#endif  // ENDIAN_H_

