#include <stdio.h>
#include <stdlib.h>

#include "api.h"
#if defined(CRYPTO_AEAD)
#include "crypto_aead.h"
#elif defined(CRYPTO_HASH)
#include "crypto_hash.h"
#endif

#if !defined(__arm__) && !defined(_M_ARM)
#pragma message("Using RDTSC to count cycles")
#ifdef _MSC_VER
#include <intrin.h>
#define ALIGN(x)
#else
#include <x86intrin.h>
#define ALIGN(x) __attribute__((aligned(x)))
#endif
#define init_cpucycles()
#define cpucycles(cycles) cycles = __rdtsc()
//#define cpucycles(cycles) cycles = __rdtscp(&tmp)
#endif

#if defined(__ARM_ARCH_6__) || __ARM_ARCH == 6 || _M_ARM == 6
#define ALIGN(x) __attribute__((aligned(x)))
#pragma message("Using ARMv6 PMU to count cycles")
#define init_cpucycles() \
  __asm__ __volatile__("mcr p15, 0, %0, c15, c12, 0" ::"r"(1))
#define cpucycles(cycles) \
  __asm__ __volatile__("mrc p15, 0, %0, c15, c12, 1" : "=r"(cycles))
#elif defined(__arm__) || defined(_M_ARM)
#define ALIGN(x) __attribute__((aligned(x)))
#pragma message("Using ARMv7 PMU to count cycles")
#define init_cpucycles()                                                \
  __asm__ __volatile__("mcr p15, 0, %0, c9, c12, 0" ::"r"(17));         \
  __asm__ __volatile__("mcr p15, 0, %0, c9, c12, 1" ::"r"(0x8000000f)); \
  __asm__ __volatile__("mcr p15, 0, %0, c9, c12, 3" ::"r"(0x8000000f))
#define cpucycles(cycles) \
  __asm__ __volatile__("mrc p15, 0, %0, c9, c13, 0" : "=r"(cycles))
#endif

#define NUM_RUNS 16
#define NUM_BYTES 32768
#define MAX_LEN 32768
#define NUM_MLENS 7

unsigned long long mlens[] = {1, 8, 16, 32, 64, 1536, 32768};
unsigned char ALIGN(16) m[MAX_LEN];
#if defined(CRYPTO_AEAD)
unsigned long long alen = 0;
unsigned long long clen = 0;
unsigned char ALIGN(16) a[MAX_LEN];
unsigned char ALIGN(16) c[MAX_LEN + CRYPTO_ABYTES];
unsigned char ALIGN(16) nsec[CRYPTO_NSECBYTES ? CRYPTO_NSECBYTES : 1];
unsigned char ALIGN(16) npub[CRYPTO_NPUBBYTES ? CRYPTO_NPUBBYTES : 1];
unsigned char ALIGN(16) k[CRYPTO_KEYBYTES];
#elif defined(CRYPTO_HASH)
unsigned char ALIGN(16) h[CRYPTO_BYTES];
#endif

unsigned long long cycles[NUM_MLENS][NUM_RUNS * 2];
unsigned int tmp;

void init_input() {
  int i;
  for (i = 0; i < MAX_LEN; ++i) m[i] = rand();
#if defined(CRYPTO_AEAD)
  for (i = 0; i < MAX_LEN; ++i) a[i] = rand();
  for (i = 0; i < CRYPTO_KEYBYTES; ++i) k[i] = rand();
  for (i = 0; i < CRYPTO_NPUBBYTES; ++i) npub[i] = rand();
#endif
}

unsigned long long measure(unsigned long long mlen) {
  unsigned long long NREPS = NUM_BYTES / mlen;
  unsigned long long i;
#if defined(__arm__) || defined(_M_ARM)
  unsigned int before, after;
#else
  unsigned long long before, after;
#endif
  init_input();
  cpucycles(before);
  for (i = 0; i < NREPS; ++i)
#if defined(CRYPTO_AEAD)
    crypto_aead_encrypt(c, &clen, m, mlen, a, alen, nsec, npub, k);
#elif defined(CRYPTO_HASH)
    crypto_hash(h, m, mlen);
#endif
  cpucycles(after);
  return after - before;
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

  init_cpucycles();

  for (i = 0; i < NUM_MLENS; ++i) {
    for (j = 0; j < NUM_RUNS; ++j) cycles[i][j] = measure(mlens[i]);
    qsort(cycles[i], NUM_RUNS, sizeof(unsigned long long), &compare_uint64);
  }

  printf("\nsorted cycles:\n");
  for (i = 0; i < NUM_MLENS; ++i) {
    unsigned long long NREPS = NUM_BYTES / mlens[i];
    printf("%5d: ", (int)mlens[i]);
    for (j = 0; j < NUM_RUNS; ++j) printf("%d ", (int)(cycles[i][j] / NREPS));
    printf("\n");
  }

  printf("\ncycles per byte (min,median):\n");
  for (i = 0; i < NUM_MLENS; ++i) {
    unsigned long long NREPS = NUM_BYTES / mlens[i];
    unsigned long long bytes = mlens[i] * NREPS;
    printf("%5d: %6.1f %6.1f\n", (int)mlens[i],
           factor * cycles[i][0] / bytes + 0.05,
           factor * cycles[i][NUM_RUNS / 2] / bytes + 0.05);
  }
  printf("\n");

  for (i = 0; i < NUM_MLENS; ++i) printf("| %5d ", (int)mlens[i]);
  printf("|\n");
  for (i = 0; i < NUM_MLENS; ++i) printf("|------:");
  printf("|\n");
  for (i = 0; i < NUM_MLENS; ++i) {
    unsigned long long NREPS = NUM_BYTES / mlens[i];
    unsigned long long bytes = mlens[i] * NREPS;
    if (mlens[i] <= 32)
      printf("| %5.0f ", factor * cycles[i][0] / bytes + 0.5);
    else
      printf("| %5.1f ", factor * cycles[i][0] / bytes + 0.05);
  }
  printf("|\n");

  return 0;
}

