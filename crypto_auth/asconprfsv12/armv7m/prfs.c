#include "api.h"
#include "ascon.h"
#include "crypto_auth.h"
#include "permutations.h"
#include "printstate.h"
#include "word.h"

#ifdef ASCON_PRF_BYTES

int crypto_prf(unsigned char* out, unsigned long long outlen,
               const unsigned char* in, unsigned long long inlen,
               const unsigned char* k) {
  if (inlen > 16 || outlen > 16 || outlen > CRYPTO_BYTES) return -1;
  /* load key */
  const uint64_t K0 = LOAD(k, 8);
  const uint64_t K1 = LOAD(k + 8, 8);
  /* initialize */
  ascon_state_t s;
  s.x[0] = ASCON_PRFS_IV ^ PRFS_MLEN(inlen);
  s.x[1] = K0;
  s.x[2] = K1;
  s.x[3] = 0;
  s.x[4] = 0;
  printstate("initial value", &s);

  /* absorb plaintext words */
  if (inlen > 8) {
    s.x[3] = LOAD(in, 8);
    s.x[4] = LOADBYTES(in + 8, inlen - 8);
  } else if (inlen) {
    s.x[3] = LOADBYTES(in, inlen);
  }
  printstate("absorb data", &s);

  /* squeeze */
  P(&s, 12);
  s.x[3] ^= K0;
  s.x[4] ^= K1;
  printstate("squeeze", &s);
  if (outlen > 8) {
    STORE(out, s.x[3], 8);
    STOREBYTES(out + 8, s.x[4], outlen - 8);
  } else if (outlen) {
    STOREBYTES(out, s.x[3], outlen);
  }
  return 0;
}

int crypto_auth(unsigned char* out, const unsigned char* in,
                unsigned long long len, const unsigned char* k) {
  return crypto_prf(out, CRYPTO_BYTES, in, len, k);
}

int crypto_auth_verify(const unsigned char* h, const unsigned char* in,
                       unsigned long long len, const unsigned char* k) {
  if (len > 16) return -1;
  int i;
  uint8_t diff = 0;
  uint8_t tag[CRYPTO_BYTES];
  crypto_prf(tag, CRYPTO_BYTES, in, len, k);
  for (i = 0; i < CRYPTO_BYTES; ++i) diff |= h[i] ^ tag[i];
  return (1 & ((diff - 1) >> 8)) - 1;
}

#endif
