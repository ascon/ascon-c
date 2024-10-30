#include "core.h"
#include "printstate.h"

int crypto_aead_encrypt(unsigned char* c, unsigned long long* clen,
                        const unsigned char* m, unsigned long long mlen,
                        const unsigned char* ad, unsigned long long adlen,
                        const unsigned char* nsec, const unsigned char* npub,
                        const unsigned char* k) {
  ascon_state_t s;
  (void)nsec;

  // set ciphertext size
  *clen = mlen + CRYPTO_ABYTES;
  print("encrypt\n");
  printbytes("k", k, CRYPTO_KEYBYTES);
  printbytes("n", npub, CRYPTO_NPUBBYTES);
  printbytes("a", ad, adlen);
  printbytes("m", m, mlen);

  ascon_core(&s, c, m, mlen, ad, adlen, npub, k, ASCON_ENC);

  // get tag
  int i;
  for (i = 0; i < CRYPTO_ABYTES; ++i) c[mlen + i] = *(s.b[3] + i);

  printbytes("c", c, mlen);
  printbytes("t", c + mlen, CRYPTO_ABYTES);
  print("\n");
  return 0;
}
