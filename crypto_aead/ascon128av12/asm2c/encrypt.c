#include "core.h"

int crypto_aead_encrypt(unsigned char* c, unsigned long long* clen,
                        const unsigned char* m, unsigned long long mlen,
                        const unsigned char* ad, unsigned long long adlen,
                        const unsigned char* nsec, const unsigned char* npub,
                        const unsigned char* k) {
  state s;
  u32_4 tmp;
  (void)nsec;

  // set ciphertext size
  *clen = mlen + CRYPTO_ABYTES;

  ascon_core(&s, c, m, mlen, ad, adlen, npub, k, ASCON_ENC);

  tmp.words[0] = s.x3;
  tmp.words[1] = s.x4;
  tmp = ascon_rev8(tmp);

  // set tag
  ((u32*)(c + mlen))[0] = tmp.words[0].h;
  ((u32*)(c + mlen))[1] = tmp.words[0].l;
  ((u32*)(c + mlen))[2] = tmp.words[1].h;
  ((u32*)(c + mlen))[3] = tmp.words[1].l;

  return 0;
}
