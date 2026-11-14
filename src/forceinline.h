#ifndef FORCEINLINE_H_
#define FORCEINLINE_H_

/* define forceinline macro */
#ifdef _MSC_VER
#define forceinline static __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define forceinline static inline __attribute__((__always_inline__))
#else
#define forceinline static inline
#endif

#endif /* FORCEINLINE_H_ */
