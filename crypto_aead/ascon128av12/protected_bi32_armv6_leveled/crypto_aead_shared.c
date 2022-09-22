#include "crypto_aead_shared.h"

#include <stdlib.h>

#include "api.h"
#include "ascon.h"
#include "shares.h"

int crypto_aead_encrypt_shared(mask_c_uint32_t* cs, unsigned long long* clen,
                               const mask_m_uint32_t* ms,
                               unsigned long long mlen,
                               const mask_ad_uint32_t* ads,
                               unsigned long long adlen,
                               const mask_npub_uint32_t* npubs,
                               const mask_key_uint32_t* ks) {
  ascon_state_t s;
  *clen = mlen + CRYPTO_ABYTES;
  ascon_initaead(&s, ks, npubs);
  if (NUM_SHARES_KEY != NUM_SHARES_AD) ascon_level_adata(&s);
  ascon_adata(&s, ads, adlen);
  if (NUM_SHARES_AD != NUM_SHARES_M) ascon_level_encdec(&s);
  ascon_encrypt(&s, cs, ms, mlen);
  if (NUM_SHARES_M != NUM_SHARES_KEY) ascon_level_final(&s);
  ascon_final(&s, ks);
  ascon_settag(&s, cs + NUM_WORDS(mlen));
  return 0;
}

int crypto_aead_decrypt_shared(mask_m_uint32_t* ms, unsigned long long* mlen,
                               const mask_c_uint32_t* cs,
                               unsigned long long clen,
                               const mask_ad_uint32_t* ads,
                               unsigned long long adlen,
                               const mask_npub_uint32_t* npubs,
                               const mask_key_uint32_t* ks) {
  ascon_state_t s;
  *mlen = clen - CRYPTO_ABYTES;
  ascon_initaead(&s, ks, npubs);
  if (NUM_SHARES_KEY != NUM_SHARES_AD) ascon_level_adata(&s);
  ascon_adata(&s, ads, adlen);
  if (NUM_SHARES_AD != NUM_SHARES_M) ascon_level_encdec(&s);
  ascon_decrypt(&s, ms, cs, *mlen);
  if (NUM_SHARES_M != NUM_SHARES_KEY) ascon_level_final(&s);
  ascon_final(&s, ks);
  ascon_xortag(&s, cs + NUM_WORDS(*mlen));
  return ascon_iszero(&s);
}
