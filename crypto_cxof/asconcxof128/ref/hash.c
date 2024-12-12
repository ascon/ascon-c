#include "api.h"
#include "ascon.h"
#include "crypto_hash.h"
#include "permutations.h"
#include "printstate.h"
#include "word.h"

int crypto_cxof(unsigned char* out, unsigned long long outlen,
                const unsigned char* in, unsigned long long inlen,
                const unsigned char* cs, unsigned long long cslen) {
  printbytes("z", cs, cslen);
  printbytes("m", in, inlen);
  /* initialize */
  ascon_state_t s;
  s.x[0] = ASCON_CXOF_IV;
  s.x[1] = 0;
  s.x[2] = 0;
  s.x[3] = 0;
  s.x[4] = 0;
  printstate("initial value", &s);
  P12(&s);
  printstate("initialization", &s);

  /* absorb customization length */
  s.x[0] ^= cslen * 8;
  printstate("absorb cs length", &s);
  P12(&s);

  /* absorb full customization blocks */
  while (cslen >= ASCON_HASH_RATE) {
    s.x[0] ^= LOADBYTES(cs, 8);
    printstate("absorb cs", &s);
    P12(&s);
    cs += ASCON_HASH_RATE;
    cslen -= ASCON_HASH_RATE;
  }
  /* absorb final customization block */
  s.x[0] ^= LOADBYTES(cs, cslen);
  s.x[0] ^= PAD(cslen);
  printstate("pad cs", &s);
  P12(&s);

  /* absorb full plaintext blocks */
  while (inlen >= ASCON_HASH_RATE) {
    s.x[0] ^= LOADBYTES(in, 8);
    printstate("absorb plaintext", &s);
    P12(&s);
    in += ASCON_HASH_RATE;
    inlen -= ASCON_HASH_RATE;
  }
  /* absorb final plaintext block */
  s.x[0] ^= LOADBYTES(in, inlen);
  s.x[0] ^= PAD(inlen);
  printstate("pad plaintext", &s);
  P12(&s);

  /* squeeze full output blocks */
  while (outlen > ASCON_HASH_RATE) {
    STOREBYTES(out, s.x[0], 8);
    printstate("squeeze output", &s);
    P12(&s);
    out += ASCON_HASH_RATE;
    outlen -= ASCON_HASH_RATE;
  }
  /* squeeze final output block */
  STOREBYTES(out, s.x[0], outlen);
  printstate("squeeze output", &s);
  printbytes("h", out + outlen - CRYPTO_BYTES, CRYPTO_BYTES);

  return 0;
}

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long len) {
  return crypto_cxof(out, CRYPTO_BYTES, in, len, (void*)0, 0);
}
