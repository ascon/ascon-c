#include "api.h"
#include "ascon.h"
#include "crypto_hash.h"
#include "permutations.h"
#include "printstate.h"
#include "word.h"

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long len) {
  /* initialize */
  state_t s;
  s.x0 = ASCON_HASHA_IV;
  s.x1 = 0;
  s.x2 = 0;
  s.x3 = 0;
  s.x4 = 0;
  P12(&s);
  printstate("initialization", &s);

  /* absorb full plaintext blocks */
  while (len >= ASCON_HASH_RATE) {
    s.x0 ^= LOADBYTES(in, 8);
    P8(&s);
    in += ASCON_HASH_RATE;
    len -= ASCON_HASH_RATE;
  }
  /* absorb final plaintext block */
  s.x0 ^= LOADBYTES(in, len);
  s.x0 ^= PAD(len);
  P12(&s);
  printstate("absorb plaintext", &s);

  /* squeeze full output blocks */
  len = CRYPTO_BYTES;
  while (len > ASCON_HASH_RATE) {
    STOREBYTES(out, s.x0, 8);
    P8(&s);
    out += ASCON_HASH_RATE;
    len -= ASCON_HASH_RATE;
  }
  /* squeeze final output block */
  STOREBYTES(out, s.x0, len);
  printstate("squeeze output", &s);

  return 0;
}
