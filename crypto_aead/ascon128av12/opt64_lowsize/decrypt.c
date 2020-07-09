#include "core.h"

int crypto_aead_decrypt(unsigned char* m, unsigned long long* mlen,
                        unsigned char* nsec, const unsigned char* c,
                        unsigned long long clen, const unsigned char* ad,
                        unsigned long long adlen, const unsigned char* npub,
                        const unsigned char* k) {
  if (clen < CRYPTO_ABYTES) {
    *mlen = 0;
    return -1;
  }

  state s;
  (void)nsec;

  /* set plaintext size */
  *mlen = clen - CRYPTO_ABYTES;

  ascon_core(&s, m, c, *mlen, ad, adlen, npub, k, ASCON_DEC);

  /* verify tag (should be constant time, check compiler output) */
  if (((s.x3 ^ LOAD64((c + *mlen))) | (s.x4 ^ LOAD64((c + *mlen + 8)))) != 0) {
    *mlen = 0;
    return -1;
  }

  return 0;
}
