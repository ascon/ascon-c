#include "api.h"
#include "ascon.h"
#include "crypto_hash.h"
#include "permutations.h"
#include "printstate.h"
#include "word.h"

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long len) {
  /* initialize */
  ascon_state_t s;
  s.x[0] = ASCON_XOFA_IV;
  s.x[1] = 0;
  s.x[2] = 0;
  s.x[3] = 0;
  s.x[4] = 0;
  printstate("initial value", &s);
  P12(&s);
  printstate("initialization", &s);

  /* absorb full plaintext blocks */
  while (len >= ASCON_HASH_RATE) {
    s.x[0] ^= LOADBYTES(in, 8);
    printstate("absorb plaintext", &s);
    P8(&s);
    in += ASCON_HASH_RATE;
    len -= ASCON_HASH_RATE;
  }
  /* absorb final plaintext block */
  s.x[0] ^= LOADBYTES(in, len);
  s.x[0] ^= PAD(len);
  printstate("pad plaintext", &s);
  P12(&s);

  /* squeeze full output blocks */
  len = CRYPTO_BYTES;
  while (len > ASCON_HASH_RATE) {
    STOREBYTES(out, s.x[0], 8);
    printstate("squeeze output", &s);
    P8(&s);
    out += ASCON_HASH_RATE;
    len -= ASCON_HASH_RATE;
  }
  /* squeeze final output block */
  STOREBYTES(out, s.x[0], len);
  printstate("squeeze output", &s);

  return 0;
}
