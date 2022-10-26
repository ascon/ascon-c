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
  int ir = (ASCON_PRF_ROUNDS == 12) ? ASCON_PRF_IN_RATE : ASCON_PRFA_IN_RATE;
  int or = ASCON_PRF_OUT_RATE;
  int nr = ASCON_PRF_ROUNDS;
  int i;
  /* load key */
  const uint64_t K0 = LOAD(k, 8);
  const uint64_t K1 = LOAD(k + 8, 8);
  /* initialize */
  ascon_state_t s;
  if (ASCON_PRF_BYTES == 0 && ASCON_PRF_ROUNDS == 12) s.x[0] = ASCON_PRF_IV;
  if (ASCON_PRF_BYTES == 0 && ASCON_PRF_ROUNDS == 8) s.x[0] = ASCON_PRFA_IV;
  if (ASCON_PRF_BYTES == 16 && ASCON_PRF_ROUNDS == 12) s.x[0] = ASCON_MAC_IV;
  if (ASCON_PRF_BYTES == 16 && ASCON_PRF_ROUNDS == 8) s.x[0] = ASCON_MACA_IV;
  if (ASCON_PRF_BYTES != 16 && ASCON_PRF_BYTES != 0)
    s.x[0] ^= U64TOWORD(((uint64_t)(12 - ASCON_PRF_ROUNDS) << 32) |
                        ASCON_PRF_BYTES * 8);
  INSERT(s.b[1], k, 8);
  INSERT(s.b[2], k + 8, 8);
  s.x[3] = 0;
  s.x[4] = 0;
  printstate("initial value", &s);
  P(&s, 12);
  printstate("initialization", &s);

  /* absorb full plaintext words */
  i = 0;
  while (inlen >= 8) {
    ABSORB(s.b[i], in, 8);
    if (++i == (ir / 8)) i = 0;
    if (i == 0) printstate("absorb plaintext", &s);
    if (i == 0) P(&s, nr);
    in += 8;
    inlen -= 8;
  }
  /* absorb final plaintext word */
  ABSORB(s.b[i], in, inlen);
  s.b[i][7 - inlen] ^= 0x80;
  printstate("domain separation", &s);
  /* domain separation */
  s.b[4][0] ^= 1;

  /* squeeze */
  printstate("domain separation", &s);
  P(&s, 12);
  /* squeeze output words */
  i = 0;
  while (outlen > 8) {
    SQUEEZE(out, s.b[i], 8);
    if (++i == or / 8) i = 0;
    if (i == 0) printstate("squeeze output", &s);
    if (i == 0) P(&s, nr);
    out += 8;
    outlen -= 8;
  }
  /* squeeze final output word */
  SQUEEZE(out, s.b[i], outlen);
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

#endif
