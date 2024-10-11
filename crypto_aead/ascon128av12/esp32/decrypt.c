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

  tmp.words[0].l = s.x3.l;
  tmp.words[0].h = s.x3.h;
  tmp.words[1].l = s.x4.l;
  tmp.words[1].h = s.x4.h;
  tmp = ascon_rev8(tmp);

  // verify should be constant time, check compiler output
  int i;
  int result = 0;
  for (i = 0; i < CRYPTO_ABYTES; ++i)
    result |= c[*mlen + i] ^ ((unsigned char*)&tmp)[i];
  result = (((result - 1) >> 8) & 1) - 1;

  return result;
}
