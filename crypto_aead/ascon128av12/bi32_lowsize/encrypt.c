#include "core.h"

int crypto_aead_encrypt(unsigned char* c, unsigned long long* clen,
                        const unsigned char* m, unsigned long long mlen,
                        const unsigned char* ad, unsigned long long adlen,
                        const unsigned char* nsec, const unsigned char* npub,
                        const unsigned char* k) {
  state s;
  u64 tmp0, tmp1;
  (void)nsec;

  /* set ciphertext size */
  *clen = mlen + CRYPTO_ABYTES;

  ascon_core(&s, c, m, mlen, ad, adlen, npub, k, ASCON_ENC);

  /* set tag */
  tmp0 = from_bit_interleaving(s.x3);
  STORE64((c + mlen), tmp0);
  tmp1 = from_bit_interleaving(s.x4);
  STORE64((c + mlen + 8), tmp1);

  return 0;
}
