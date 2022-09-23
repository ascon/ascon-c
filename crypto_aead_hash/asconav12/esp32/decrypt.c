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
  u32_4 tmp;
  (void)nsec;

  // set plaintext size
  *mlen = clen - CRYPTO_ABYTES;

  ascon_core(&s, m, c, *mlen, ad, adlen, npub, k, ASCON_DEC);

  tmp.words[0].h = ((u32*)(c + *mlen))[0];
  tmp.words[0].l = ((u32*)(c + *mlen))[1];
  tmp.words[1].h = ((u32*)(c + *mlen))[2];
  tmp.words[1].l = ((u32*)(c + *mlen))[3];
  tmp = ascon_rev8(tmp);
  u32_2 t0 = tmp.words[0];
  u32_2 t1 = tmp.words[1];

  // verify tag (should be constant time, check compiler output)
  if (((s.x3.h ^ t0.h) | (s.x3.l ^ t0.l) | (s.x4.h ^ t1.h) | (s.x4.l ^ t1.l)) !=
      0) {
    *mlen = 0;
    return -1;
  }

  return 0;
}
