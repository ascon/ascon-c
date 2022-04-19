#ifndef SHARES_H_
#define SHARES_H_

#include <stdint.h>

#include "api.h"
#include "config.h"

#define NUM_WORDS(len) ((((len) + 7) / 8) * 2)
#define ROT(i) (((i) * (ASCON_ROR_SHARES)) & 31)

typedef struct {
  uint32_t shares[NUM_SHARES_M];
} mask_m_uint32_t;

typedef struct {
  uint32_t shares[NUM_SHARES_C];
} mask_c_uint32_t;

typedef struct {
  uint32_t shares[NUM_SHARES_AD];
} mask_ad_uint32_t;

typedef struct {
  uint32_t shares[NUM_SHARES_NPUB];
} mask_npub_uint32_t;

typedef struct {
  uint32_t shares[NUM_SHARES_KEY];
} mask_key_uint32_t;

void generate_shares_encrypt(const unsigned char* m, mask_m_uint32_t* ms,
                             const unsigned long long mlen,
                             const unsigned char* ad, mask_ad_uint32_t* ads,
                             const unsigned long long adlen,
                             const unsigned char* npub,
                             mask_npub_uint32_t* npubs, const unsigned char* k,
                             mask_key_uint32_t* ks);

void generate_shares_decrypt(const unsigned char* c, mask_c_uint32_t* cs,
                             const unsigned long long clen,
                             const unsigned char* ad, mask_ad_uint32_t* ads,
                             const unsigned long long adlen,
                             const unsigned char* npub,
                             mask_npub_uint32_t* npubs, const unsigned char* k,
                             mask_key_uint32_t* ks);

void combine_shares_encrypt(const mask_c_uint32_t* cs, unsigned char* c,
                            unsigned long long clen);

void combine_shares_decrypt(const mask_m_uint32_t* ms, unsigned char* m,
                            unsigned long long mlen);

#endif /* SHARES_H_ */
