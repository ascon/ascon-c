#include "api.h"
#include "ascon.h"
#include "crypto_aead.h"
#include "permutations.h"
#include "printstate.h"

void ascon_aead(state_t* s, uint8_t* out, const uint8_t* in, uint64_t tlen,
                const uint8_t* ad, uint64_t adlen, const uint8_t* npub,
                const uint8_t* k, uint8_t mode);

int crypto_aead_encrypt(unsigned char* c, unsigned long long* clen,
                        const unsigned char* m, unsigned long long mlen,
                        const unsigned char* ad, unsigned long long adlen,
                        const unsigned char* nsec, const unsigned char* npub,
                        const unsigned char* k) {
  state_t s;
  (void)nsec;
  /* set ciphertext size */
  *clen = mlen + CRYPTO_ABYTES;
  /* ascon encryption */
  ascon_aead(&s, c, m, mlen, ad, adlen, npub, k, ASCON_ENCRYPT);
  /* set tag */
  STOREBYTES(c + mlen, s.x3, 8);
  STOREBYTES(c + mlen + 8, s.x4, 8);
  return 0;
}
