#include "api.h"
#include "ascon.h"
#include "crypto_aead.h"
#include "permutations.h"
#include "printstate.h"
#include "word.h"

int crypto_aead_encrypt(unsigned char* c, unsigned long long* clen,
                        const unsigned char* m, unsigned long long mlen,
                        const unsigned char* ad, unsigned long long adlen,
                        const unsigned char* nsec, const unsigned char* npub,
                        const unsigned char* k) {
  (void)nsec;

  /* set ciphertext size */
  *clen = mlen + CRYPTO_ABYTES;

  /* load key and nonce */
  const uint64_t K0 = LOADBYTES(k, 8);
  const uint64_t K1 = LOADBYTES(k + 8, 8);
  const uint64_t N0 = LOADBYTES(npub, 8);
  const uint64_t N1 = LOADBYTES(npub + 8, 8);

  /* initialize */
  state_t s;
  s.x0 = ASCON_128A_IV;
  s.x1 = K0;
  s.x2 = K1;
  s.x3 = N0;
  s.x4 = N1;
  P12(&s);
  s.x3 ^= K0;
  s.x4 ^= K1;
  printstate("initialization", &s);

  if (adlen) {
    /* full associated data blocks */
    while (adlen >= ASCON_128A_RATE) {
      s.x0 ^= LOADBYTES(ad, 8);
      s.x1 ^= LOADBYTES(ad + 8, 8);
      P8(&s);
      ad += ASCON_128A_RATE;
      adlen -= ASCON_128A_RATE;
    }
    /* final associated data block */
    if (adlen >= 8) {
      s.x0 ^= LOADBYTES(ad, 8);
      s.x1 ^= LOADBYTES(ad + 8, adlen - 8);
      s.x1 ^= PAD(adlen - 8);
    } else {
      s.x0 ^= LOADBYTES(ad, adlen);
      s.x0 ^= PAD(adlen);
    }
    P8(&s);
  }
  /* domain separation */
  s.x4 ^= 1;
  printstate("process associated data", &s);

  /* full plaintext blocks */
  while (mlen >= ASCON_128A_RATE) {
    s.x0 ^= LOADBYTES(m, 8);
    s.x1 ^= LOADBYTES(m + 8, 8);
    STOREBYTES(c, s.x0, 8);
    STOREBYTES(c + 8, s.x1, 8);
    P8(&s);
    m += ASCON_128A_RATE;
    c += ASCON_128A_RATE;
    mlen -= ASCON_128A_RATE;
  }
  /* final plaintext block */
  if (mlen >= 8) {
    s.x0 ^= LOADBYTES(m, 8);
    s.x1 ^= LOADBYTES(m + 8, mlen - 8);
    STOREBYTES(c, s.x0, 8);
    STOREBYTES(c + 8, s.x1, mlen - 8);
    s.x1 ^= PAD(mlen - 8);
  } else {
    s.x0 ^= LOADBYTES(m, mlen);
    STOREBYTES(c, s.x0, mlen);
    s.x0 ^= PAD(mlen);
  }
  c += mlen;
  printstate("process plaintext", &s);

  /* finalize */
  s.x2 ^= K0;
  s.x3 ^= K1;
  P12(&s);
  s.x3 ^= K0;
  s.x4 ^= K1;
  printstate("finalization", &s);

  /* set tag */
  STOREBYTES(c, s.x3, 8);
  STOREBYTES(c + 8, s.x4, 8);

  return 0;
}
