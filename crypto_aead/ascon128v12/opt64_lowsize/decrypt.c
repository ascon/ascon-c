#include "api.h"
#include "ascon.h"
#include "crypto_aead.h"
#include "permutations.h"
#include "printstate.h"

void ascon_aead(state_t* s, uint8_t* out, const uint8_t* in, uint64_t tlen,
                const uint8_t* ad, uint64_t adlen, const uint8_t* npub,
                const uint8_t* k, uint8_t mode);

int crypto_aead_decrypt(unsigned char* m, unsigned long long* mlen,
                        unsigned char* nsec, const unsigned char* c,
                        unsigned long long clen, const unsigned char* ad,
                        unsigned long long adlen, const unsigned char* npub,
                        const unsigned char* k) {
  state_t s;
  (void)nsec;
  if (clen < CRYPTO_ABYTES) return -1;
  /* set plaintext size */
  *mlen = clen - CRYPTO_ABYTES;
  /* ascon decryption */
  ascon_aead(&s, m, c, *mlen, ad, adlen, npub, k, ASCON_DECRYPT);
  /* verify tag (should be constant time, check compiler output) */
  s.x3 = XOR(s.x3, LOADBYTES(c + *mlen, 8));
  s.x4 = XOR(s.x4, LOADBYTES(c + *mlen + 8, 8));
  return NOTZERO(s.x3, s.x4);
}
