#include <stdio.h>
#include <string.h>

#include "api.h"
#if defined(CRYPTO_AEAD)
#include "crypto_aead.h"
#elif defined(CRYPTO_HASH)
#include "crypto_hash.h"
#endif

void print(unsigned char c, unsigned char* x, unsigned long long xlen) {
  unsigned long long i;
  printf("%c[%d]=", c, (int)xlen);
  for (i = 0; i < xlen; ++i) printf("%02x", x[i]);
  printf("\n");
}

int main() {
  int result = 0;
#if defined(CRYPTO_AEAD)
  unsigned long long alen = 0;
  unsigned long long mlen = 0;
  unsigned long long clen = CRYPTO_ABYTES;
  unsigned char a[] = "ASCON";
  unsigned char m[] = "ascon";
  unsigned char c[strlen((const char*)m) + CRYPTO_ABYTES];
  unsigned char nsec[CRYPTO_NSECBYTES];
  unsigned char npub[CRYPTO_NPUBBYTES] = {0};
  unsigned char k[CRYPTO_KEYBYTES] = {0};
  alen = strlen((const char*)a);
  mlen = strlen((const char*)m);
  print('k', k, CRYPTO_KEYBYTES);
  printf(" ");
  print('n', npub, CRYPTO_NPUBBYTES);
  printf("\n");
  print('a', a, alen);
  printf(" ");
  print('m', m, mlen);
  printf(" -> ");
  result |= crypto_aead_encrypt(c, &clen, m, mlen, a, alen, nsec, npub, k);
  print('c', c, clen - CRYPTO_ABYTES);
  printf(" ");
  print('t', c + clen - CRYPTO_ABYTES, CRYPTO_ABYTES);
  printf(" -> ");
  result |= crypto_aead_decrypt(m, &mlen, nsec, c, clen, a, alen, npub, k);
  print('a', a, alen);
  printf(" ");
  print('m', m, mlen);
  printf("\n");
#elif defined(CRYPTO_HASH)
  unsigned long long mlen = 0;
  unsigned char m[] = "ascon";
  unsigned char h[CRYPTO_BYTES] = {0};
  mlen = strlen((const char*)m);
  print('m', m, mlen);
  printf(" -> ");
  result |= crypto_hash(h, m, mlen);
  print('h', h, CRYPTO_BYTES);
  printf("\n");
#endif
  return result;
}

