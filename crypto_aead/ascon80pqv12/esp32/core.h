#ifndef CORE_H_
#define CORE_H_

#include "api.h"
#include "lendian.h"
#include "permutations.h"

#define ASCON_AD 0
#define ASCON_ENC 1
#define ASCON_DEC 2

void process_data(ascon_state_t* s, unsigned char* out, const unsigned char* in,
                  unsigned long long len, uint8_t mode);

void ascon_core(ascon_state_t* s, unsigned char* out, const unsigned char* in,
                unsigned long long tlen, const unsigned char* ad,
                unsigned long long adlen, const unsigned char* npub,
                const unsigned char* k, uint8_t mode);

#endif  // CORE_H_
