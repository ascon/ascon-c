#define CRYPTO_VERSION "1.3.0"
#define CRYPTO_KEYBYTES 16
#define CRYPTO_NSECBYTES 0
#define CRYPTO_NPUBBYTES 16
#define CRYPTO_ABYTES 16
#define CRYPTO_NOOVERLAP 1
#define ASCON_AEAD_RATE 16

#ifndef ASCON_ROR_SHARES
#define ASCON_ROR_SHARES 5
#endif

#ifndef NUM_SHARES_M
#define NUM_SHARES_M 1
#endif

#ifndef NUM_SHARES_C
#define NUM_SHARES_C 1
#endif

#ifndef NUM_SHARES_AD
#define NUM_SHARES_AD 1
#endif

#ifndef NUM_SHARES_NPUB
#define NUM_SHARES_NPUB 1
#endif

#ifndef NUM_SHARES_KEY
#define NUM_SHARES_KEY 2
#endif
#define ASCON_VARIANT 1
