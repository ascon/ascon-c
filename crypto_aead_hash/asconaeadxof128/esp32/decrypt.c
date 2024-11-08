#include "core.h"
#include "printstate.h"

int crypto_aead_decrypt(unsigned char* m, unsigned long long* mlen,
                        unsigned char* nsec, const unsigned char* c,
                        unsigned long long clen, const unsigned char* ad,
                        unsigned long long adlen, const unsigned char* npub,
                        const unsigned char* k) {
  if (clen < CRYPTO_ABYTES) {
    *mlen = 0;
    return -1;
  }

  ascon_state_t s;
  (void)nsec;

  // set plaintext size
  *mlen = clen - CRYPTO_ABYTES;
  print("decrypt\n");
  printbytes("k", k, CRYPTO_KEYBYTES);
  printbytes("n", npub, CRYPTO_NPUBBYTES);
  printbytes("a", ad, adlen);
  printbytes("c", c, *mlen);
  printbytes("t", c + *mlen, CRYPTO_ABYTES);

  ascon_core(&s, m, c, *mlen, ad, adlen, npub, k, ASCON_DEC);

  // verify should be constant time, check compiler output
  int i;
  int result = 0;
  for (i = 0; i < CRYPTO_ABYTES; ++i) result |= c[*mlen + i] ^ *(s.b[3] + i);
  result = (((result - 1) >> 8) & 1) - 1;

  printbytes("m", m, *mlen);
  print("\n");
  return result;
}
