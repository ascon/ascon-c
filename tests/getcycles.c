#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "api.h"
#include "cpucycles.h"

#if defined(AVR_UART)
#include "avr_uart.h"
#endif

#if defined(CRYPTO_AEAD_SHARED)
#include "crypto_aead_shared.h"
#elif defined(CRYPTO_AEAD)
#include "crypto_aead.h"
#elif defined(CRYPTO_HASH)
#include "crypto_hash.h"
#elif defined(CRYPTO_AUTH)
#include "crypto_auth.h"
#endif

#define NUM_RUNS 16
#define NUM_MLENS 7
#if !defined(__AVR__)
#define MAX_LEN (1 << 11)
#else
#define MAX_LEN (1 << 8)
#endif

#if defined(CRYPTO_AEAD_SHARED)
unsigned long long alen = 0;
unsigned long long clen = 0;
mask_ad_uint32_t ALIGN(16) a[NUM_WORDS(MAX_LEN)];
mask_m_uint32_t ALIGN(16) m[NUM_WORDS(MAX_LEN)];
mask_c_uint32_t ALIGN(16) c[NUM_WORDS(MAX_LEN + CRYPTO_ABYTES)];
mask_npub_uint32_t ALIGN(16) n[NUM_WORDS(CRYPTO_NPUBBYTES)];
mask_key_uint32_t ALIGN(16) k[NUM_WORDS(CRYPTO_KEYBYTES)];
#elif defined(CRYPTO_AEAD)
unsigned long long alen = 0;
unsigned long long clen = 0;
unsigned char ALIGN(16) a[MAX_LEN];
unsigned char ALIGN(16) m[MAX_LEN];
unsigned char ALIGN(16) c[MAX_LEN + CRYPTO_ABYTES];
unsigned char ALIGN(16) n[CRYPTO_NPUBBYTES];
unsigned char ALIGN(16) k[CRYPTO_KEYBYTES];
#elif defined(CRYPTO_HASH)
unsigned char ALIGN(16) m[MAX_LEN];
unsigned char ALIGN(16) h[CRYPTO_BYTES];
#elif defined(CRYPTO_AUTH)
unsigned char ALIGN(16) k[CRYPTO_KEYBYTES];
unsigned char ALIGN(16) m[MAX_LEN];
unsigned char ALIGN(16) t[CRYPTO_BYTES];
#endif

#if !defined(__AVR__)
unsigned long long mlens[] = {1, 8, 16, 32, 64, 1536, MAX_LEN};
#else
unsigned long long mlens[] = {1, 8, 16, 32, 64, 128, 256};
#endif
unsigned long long cycles[NUM_MLENS][NUM_RUNS * 2];

void init_input() {
  int i, j;
#if defined(CRYPTO_AEAD_SHARED)
  for (i = 0; i < NUM_WORDS(MAX_LEN); ++i)
    for (j = 0; j < NUM_SHARES_AD; ++j) a[i].shares[j] = rand();
  for (i = 0; i < NUM_WORDS(MAX_LEN); ++i)
    for (j = 0; j < NUM_SHARES_M; ++j) m[i].shares[j] = rand();
  for (i = 0; i < NUM_WORDS(CRYPTO_KEYBYTES); ++i)
    for (j = 0; j < NUM_SHARES_KEY; ++j) k[i].shares[j] = rand();
  for (i = 0; i < NUM_WORDS(CRYPTO_NPUBBYTES); ++i)
    for (j = 0; j < NUM_SHARES_NPUB; ++j) n[i].shares[j] = rand();
#elif defined(CRYPTO_AEAD)
  for (i = 0; i < MAX_LEN; ++i) a[i] = rand();
  for (i = 0; i < MAX_LEN; ++i) m[i] = rand();
  for (i = 0; i < CRYPTO_KEYBYTES; ++i) k[i] = rand();
  for (i = 0; i < CRYPTO_NPUBBYTES; ++i) n[i] = rand();
#else
  for (i = 0; i < MAX_LEN; ++i) m[i] = rand();
#endif
}

unsigned long long measure(unsigned long long mlen) {
  unsigned long long NREPS = MAX_LEN / mlen;
  unsigned long long i;
  int result = 0;
  init_input();
  cpucycles_reset();
  cpucycles_start();
  for (i = 0; i < NREPS; ++i) {
#if defined(CRYPTO_AEAD_SHARED)
    result |= crypto_aead_encrypt_shared(c, &clen, m, mlen, a, alen, n, k);
#elif defined(CRYPTO_AEAD)
    result |= crypto_aead_encrypt(c, &clen, m, mlen, a, alen, NULL, n, k);
#elif defined(CRYPTO_HASH)
    result |= crypto_hash(h, m, mlen);
#elif defined(CRYPTO_AUTH)
    result |= crypto_auth(t, m, mlen, k);
#endif
  }
  cpucycles_stop();
  if (!result)
    return cpucycles_result();
  else
    return -1;
}

int compare_uint64(const void* first, const void* second) {
  const unsigned long long* ia = (const unsigned long long*)first;
  const unsigned long long* ib = (const unsigned long long*)second;
  if (*ia > *ib) return 1;
  if (*ia < *ib) return -1;
  return 0;
}

int main(int argc, char* argv[]) {
  unsigned long long i, j;
  double factor = 1.0;
  if (argc == 2) factor = atof(argv[1]);

#if defined(AVR_UART)
  avr_uart_init();
  stdout = &avr_uart_output;
  stdin = &avr_uart_input_echo;
#endif
  cpucycles_init();

  for (i = 0; i < NUM_MLENS; ++i) {
    for (j = 0; j < NUM_RUNS; ++j) cycles[i][j] = measure(mlens[i]);
    qsort(cycles[i], NUM_RUNS, sizeof(unsigned long long), &compare_uint64);
  }

  printf("\nsorted cycles:\n");
  for (i = 0; i < NUM_MLENS; ++i) {
    unsigned long long NREPS = MAX_LEN / mlens[i];
    printf("%5" PRIu32 ": ", (uint32_t)mlens[i]);
    for (j = 0; j < NUM_RUNS; ++j)
      if (cycles[i][j] == -1)
        printf(" - ");
      else
        printf("%" PRIu32 " ", (uint32_t)(cycles[i][j] / NREPS));
    printf("\n");
  }

  printf("\ncycles per byte (min,median):\n");
  for (i = 0; i < NUM_MLENS; ++i) {
    unsigned long long NREPS = MAX_LEN / mlens[i];
    unsigned long long bytes = mlens[i] * NREPS;
    printf("%5" PRIu32 ": ", (uint32_t)mlens[i]);
    if (cycles[i][0] == -1)
      printf("   -      -   \n");
    else
#if !defined(__AVR__)
      printf("%6.1f %6.1f\n", factor * cycles[i][0] / bytes + 0.05,
             factor * cycles[i][NUM_RUNS / 2] / bytes + 0.05);
#else
      printf("%6" PRIu32 " %6" PRIu32 "\n",
             (uint32_t)(factor * cycles[i][0] / bytes + 0.05),
             (uint32_t)(factor * cycles[i][NUM_RUNS / 2] / bytes + 0.05));
#endif
  }
  printf("\n");

  for (i = 0; i < NUM_MLENS; ++i) printf("| %5" PRIu32 " ", (uint32_t)mlens[i]);
  printf("|\n");
  for (i = 0; i < NUM_MLENS; ++i) printf("|------:");
  printf("|\n");
  for (i = 0; i < NUM_MLENS; ++i) {
    unsigned long long NREPS = MAX_LEN / mlens[i];
    unsigned long long bytes = mlens[i] * NREPS;
    if (cycles[i][0] == -1)
      printf("|   -   ");
    else if ((mlens[i] <= 32) || (factor * cycles[i][0] / bytes + 0.5 >= 1000))
      printf("| %5" PRIu32 " ",
             (uint32_t)(factor * cycles[i][0] / bytes + 0.5));
    else
#if !defined(__AVR__)
      printf("| %5.1f ", factor * cycles[i][0] / bytes + 0.05);
#else
      printf("| %5" PRIu32 " ",
             (uint32_t)(factor * cycles[i][0] / bytes + 0.05));
#endif
  }
  printf("|\n");

  return 0;
}
