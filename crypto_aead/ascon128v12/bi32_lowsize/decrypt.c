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
  u32_2 t0, t1;
  (void)nsec;

  /* set plaintext size */
  *mlen = clen - CRYPTO_ABYTES;

  ascon_core(&s, m, c, *mlen, ad, adlen, npub, k, ASCON_DEC);

  /* verify tag (should be constant time, check compiler output) */
  t0 = to_bit_interleaving(LOAD64((c + *mlen)));
  t1 = to_bit_interleaving(LOAD64((c + *mlen + 8)));
  if (((s.x3.e ^ t0.e) | (s.x3.o ^ t0.o) | (s.x4.e ^ t1.e) | (s.x4.o ^ t1.o)) !=
      0) {
    *mlen = 0;
    return -1;
  }

  return 0;
}
