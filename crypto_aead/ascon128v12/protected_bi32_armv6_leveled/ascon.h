#ifndef ASCON_H_
#define ASCON_H_

#include <stdint.h>

#include "shares.h"
#include "word.h"

typedef struct {
  word_t x[6];
} ascon_state_t;

void ascon_initaead(ascon_state_t* s, const mask_key_uint32_t* k,
                    const mask_npub_uint32_t* n);
void ascon_adata(ascon_state_t* s, const mask_ad_uint32_t* ad, uint64_t adlen);
void ascon_encrypt(ascon_state_t* s, mask_c_uint32_t* c,
                   const mask_m_uint32_t* m, uint64_t mlen);
void ascon_decrypt(ascon_state_t* s, mask_m_uint32_t* m,
                   const mask_c_uint32_t* c, uint64_t clen);
void ascon_final(ascon_state_t* s, const mask_key_uint32_t* k);
void ascon_settag(ascon_state_t* s, mask_c_uint32_t* t);
void ascon_xortag(ascon_state_t* s, const mask_c_uint32_t* t);
int ascon_iszero(ascon_state_t* s);

void ascon_level_adata(ascon_state_t* s);
void ascon_level_encdec(ascon_state_t* s);
void ascon_level_final(ascon_state_t* s);

#endif /* ASCON_H_ */
