#include "api.h"
#include "ascon.h"
#include "crypto_auth.h"
#include "permutations.h"
#include "printstate.h"
#include "word.h"

int crypto_prf(unsigned char* out, unsigned long long outlen,
               const unsigned char* in, unsigned long long inlen,
               const unsigned char* k) {
  if (CRYPTO_BYTES && outlen > CRYPTO_BYTES) return -1;
  /* load key */
  const uint64_t K0 = LOADBYTES(k, 8);
  const uint64_t K1 = LOADBYTES(k + 8, 8);
  int i;
  /* initialize */
  ascon_state_t s;
  s.x[0] = ASCON_MAC_IV;
  s.x[1] = K0;
  s.x[2] = K1;
  s.x[3] = 0;
  s.x[4] = 0;
  printstate("initial value", &s);
  P12(&s);
  printstate("initialization", &s);

  /* absorb full plaintext words */
  i = 0;
  while (inlen >= 8) {
    ((uint64_t*)(&s.x[0]))[i] ^= LOADBYTES(in, 8);
    if (++i == 4) i = 0;
    if (i == 0) printstate("absorb plaintext", &s);
    if (i == 0) P12(&s);
    in += 8;
    inlen -= 8;
  }
  /* absorb final plaintext word */
  ((uint64_t*)(&s.x[0]))[i] ^= LOADBYTES(in, inlen);
  ((uint64_t*)(&s.x[0]))[i] ^= PAD(inlen);
  printstate("pad plaintext", &s);
  /* domain separation */
  s.x[4] ^= 1;
  printstate("domain separation", &s);

  /* squeeze */
  P12(&s);
  /* squeeze output words */
  i = 0;
  while (outlen > 8) {
    STOREBYTES(out, ((uint64_t*)(&s.x[0]))[i], 8);
    if (++i == 2) i = 0;
    if (i == 0) printstate("squeeze output", &s);
    if (i == 0) P12(&s);
    out += 8;
    outlen -= 8;
  }
  /* squeeze final output word */
  STOREBYTES(out, ((uint64_t*)(&s.x[0]))[i], outlen);
  printstate("squeeze output", &s);
  return 0;
}

int crypto_auth(unsigned char* out, const unsigned char* in,
                unsigned long long len, const unsigned char* k) {
  return crypto_prf(out, CRYPTO_BYTES, in, len, k);
}

int crypto_auth_verify(const unsigned char* h, const unsigned char* in,
                       unsigned long long len, const unsigned char* k) {
  int i;
  uint8_t diff = 0;
  uint8_t tag[CRYPTO_BYTES];
  crypto_prf(tag, CRYPTO_BYTES, in, len, k);
  for (i = 0; i < CRYPTO_BYTES; ++i) diff |= h[i] ^ tag[i];
  return (1 & ((diff - 1) >> 8)) - 1;
}
