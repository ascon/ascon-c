#ifndef CPUCYCLES_H_
#define CPUCYCLES_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if !defined(__arm__) && !defined(_M_ARM) && !defined(__AVR__)
#ifdef PRAGMA_GETCYCLES
#pragma message("Using RDTSC to count cycles")
#endif
#ifdef _MSC_VER
#include <intrin.h>
#define ALIGN(x)
#else
#include <x86intrin.h>
#define ALIGN(x) __attribute__((aligned(x)))
#endif
#define cpucycles_init()
#define cpucycles(cycles) cycles = __rdtsc()
//#define cpucycles(cycles) cycles = __rdtscp(&tmp)
#endif

#if defined(__ARM_ARCH_6__) || __ARM_ARCH == 6 || _M_ARM == 6
#define ALIGN(x) __attribute__((aligned(x)))
#ifdef PRAGMA_GETCYCLES
#pragma message("Using ARMv6 PMU to count cycles")
#endif
#define cpucycles_init() \
  __asm__ __volatile__("mcr p15, 0, %0, c15, c12, 0" ::"r"(1))
#define cpucycles(cycles) \
  __asm__ __volatile__("mrc p15, 0, %0, c15, c12, 1" : "=r"(cycles))
#elif defined(__arm__) || defined(_M_ARM)
#define ALIGN(x) __attribute__((aligned(x)))
#ifdef PRAGMA_GETCYCLES
#pragma message("Using ARMv7 PMU to count cycles")
#endif
#define cpucycles_init()                                                \
  __asm__ __volatile__("mcr p15, 0, %0, c9, c12, 0" ::"r"(17));         \
  __asm__ __volatile__("mcr p15, 0, %0, c9, c12, 1" ::"r"(0x8000000f)); \
  __asm__ __volatile__("mcr p15, 0, %0, c9, c12, 3" ::"r"(0x8000000f))
#define cpucycles(cycles) \
  __asm__ __volatile__("mrc p15, 0, %0, c9, c13, 0" : "=r"(cycles))
#endif

#if defined(__AVR__)
#ifdef PRAGMA_GETCYCLES
#pragma message("Using TCNT1 to count cycles")
#endif
#include "avr/io.h"
#define ALIGN(x)
#define cpucycles_init() TCCR1B = 5
#define cpucycles(cycles) cycles = (uint32_t)1024 * (TCNT1L + 256 * TCNT1H)
#endif

#define cpucycles_reset() cpucycles_sum = 0

#define cpucycles_start() cpucycles(cpucycles_before)

#define cpucycles_stop()                                 \
  do {                                                   \
    cpucycles(cpucycles_after);                          \
    cpucycles_sum += cpucycles_after - cpucycles_before; \
  } while (0)

#define cpucycles_result() cpucycles_sum

unsigned long long cpucycles_sum;

#if defined(__arm__) || defined(_M_ARM) || defined(__AVR__)
uint32_t cpucycles_before, cpucycles_after;
#else
unsigned long long cpucycles_before, cpucycles_after;
#endif

#endif  // CPUCYCLES_H_
