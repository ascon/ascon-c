#include "api.h"
#include "ascon.h"

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long inlen) {
  return ascon_core(out, CRYPTO_BYTES, in, inlen);
}

