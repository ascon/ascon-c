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
  if (CRYPTO_BYTES && outlen > CRYPTO_BYTES) return -1;
  /* load key */
  const word_t K0 = LOAD(k, 8);
  const word_t K1 = LOAD(k + 8, 8);
  /* initialize */
  ascon_state_t s;
  if (ASCON_PRF_BYTES == 0) s.x[0] = WORD_T(ASCON_PRF_IV);
  if (ASCON_PRF_BYTES == 16) s.x[0] = WORD_T(ASCON_MAC_IV);
  if (ASCON_PRF_BYTES != 16)
    s.x[0] = XOR(WORD_T(ASCON_PRF_IV), U64TOWORD(ASCON_PRF_BYTES * 8));
  s.x[1] = K0;
  s.x[2] = K1;
  s.x[3] = WORD_T(0);
  s.x[4] = WORD_T(0);
  printstate("initial value", &s);
  P(&s, 12);
  printstate("initialization", &s);

  /* absorb full plaintext words */
  int i = 0;
  while (inlen >= 8) {
    s.x[i] = XOR(s.x[i], LOAD(in, 8));
    if (++i == 4) i = 0;
    if (i == 0) P(&s, 12);
    in += 8;
    inlen -= 8;
  }
  /* absorb final plaintext word */
  if (inlen) s.x[i] = XOR(s.x[i], LOAD(in, inlen));
  s.x[i] = XOR(s.x[i], PAD(inlen));
  /* domain separation */
  s.x[4] = XOR(s.x[4], WORD_T(1));
  printstate("domain separation", &s);

  /* squeeze */
  P(&s, 12);
  printstate("absorb plaintext", &s);
  /* squeeze output words */
  i = 0;
  while (outlen > 8) {
    STORE(out, s.x[i], 8);
    if (++i == 2) i = 0;
    if (i == 0) P(&s, 12);
    out += 8;
    outlen -= 8;
  }
  /* squeeze final output word */
  STORE(out, s.x[i], outlen);
  printstate("squeeze output", &s);
  return 0;
}

int crypto_auth(unsigned char* out, const unsigned char* in,
                unsigned long long len, const unsigned char* k) {
  return crypto_prf(out, CRYPTO_BYTES, in, len, k);
}

int crypto_auth_verify(const unsigned char* h, const unsigned char* in,
                       unsigned long long len, const unsigned char* k) {
  uint8_t tag[CRYPTO_BYTES];
  crypto_prf(tag, CRYPTO_BYTES, in, len, k);
  uint8_t diff = 0;
  for (int i = 0; i < CRYPTO_BYTES; ++i) diff |= h[i] ^ tag[i];
  return (1 & ((diff - 1) >> 8)) - 1;
}

#endif
