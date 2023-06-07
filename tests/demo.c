#include <stdio.h>
#include <string.h>

#include "api.h"
#if defined(AVR_UART)
#include "avr_uart.h"
#endif
#if defined(CRYPTO_AEAD)
#include "crypto_aead.h"
#elif defined(CRYPTO_HASH)
#include "crypto_hash.h"
#elif defined(CRYPTO_AUTH)
#include "crypto_auth.h"
#endif

void print(unsigned char c, unsigned char* x, unsigned long long xlen) {
  unsigned long long i;
  printf("%c[%d]=", c, (int)xlen);
  for (i = 0; i < xlen; ++i) printf("%02x", x[i]);
  printf("\n");
}

int main() {
  unsigned char n[CRYPTO_NPUBBYTES] = {0, 1, 2,  3,  4,  5,  6,  7,
                                       8, 9, 10, 11, 12, 13, 14, 15};
  unsigned char k[CRYPTO_KEYBYTES] = {0, 1, 2,  3,  4,  5,  6,  7,
                                      8, 9, 10, 11, 12, 13, 14, 15};
  unsigned char a[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  unsigned char m[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  unsigned char c[32], h[32], t[32];
  unsigned long long alen = 16;
  unsigned long long mlen = 16;
  unsigned long long clen = CRYPTO_ABYTES;
  int result = 0;
#if defined(AVR_UART)
  avr_uart_init();
  stdout = &avr_uart_output;
  stdin = &avr_uart_input_echo;
#endif
#if defined(CRYPTO_AEAD)
  print('k', k, CRYPTO_KEYBYTES);
  printf(" ");
  print('n', n, CRYPTO_NPUBBYTES);
  printf("\n");
  print('a', a, alen);
  printf(" ");
  print('m', m, mlen);
  printf(" -> ");
  result |= crypto_aead_encrypt(c, &clen, m, mlen, a, alen, (void*)0, n, k);
  print('c', c, clen - CRYPTO_ABYTES);
  printf(" ");
  print('t', c + clen - CRYPTO_ABYTES, CRYPTO_ABYTES);
  printf(" -> ");
  result |= crypto_aead_decrypt(m, &mlen, (void*)0, c, clen, a, alen, n, k);
  print('a', a, alen);
  printf(" ");
  print('m', m, mlen);
  printf("\n");
#elif defined(CRYPTO_HASH)
  print('m', m, mlen);
  printf(" -> ");
  result |= crypto_hash(h, m, mlen);
  print('h', h, CRYPTO_BYTES);
  printf("\n");
#elif defined(CRYPTO_AUTH)
  print('k', k, CRYPTO_KEYBYTES);
  printf(" ");
  print('m', m, mlen);
  printf(" -> ");
  result |= crypto_auth(t, m, mlen, k);
  print('h', t, CRYPTO_BYTES);
  printf("\n");
#endif
  return result;
}
