#include "crypto_aead.h"

#include <stdlib.h>

#include "api.h"
#include "crypto_aead_shared.h"
#include "printstate.h"

#ifdef SS_VER
#include "hal.h"
#else
#define trigger_high()
#define trigger_low()
#endif

int crypto_aead_encrypt(unsigned char* c, unsigned long long* clen,
                        const unsigned char* m, unsigned long long mlen,
                        const unsigned char* a, unsigned long long alen,
                        const unsigned char* nsec, const unsigned char* npub,
                        const unsigned char* k) {
  (void)nsec;
  print("encrypt\n");
  printbytes("k", k, CRYPTO_KEYBYTES);
  printbytes("n", npub, CRYPTO_NPUBBYTES);
  printbytes("a", a, alen);
  printbytes("m", m, mlen);
  /* dynamic allocation of input/output shares */
  mask_key_uint32_t* ks = malloc(sizeof(*ks) * NUM_WORDS(CRYPTO_KEYBYTES));
  mask_npub_uint32_t* ns = malloc(sizeof(*ns) * NUM_WORDS(CRYPTO_NPUBBYTES));
  mask_ad_uint32_t* as = malloc(sizeof(*as) * NUM_WORDS(alen));
  mask_m_uint32_t* ms = malloc(sizeof(*ms) * NUM_WORDS(mlen));
  mask_c_uint32_t* cs = malloc(sizeof(*cs) * NUM_WORDS(mlen + CRYPTO_ABYTES));
  /* mask plain input data */
  generate_shares_encrypt(m, ms, mlen, a, as, alen, npub, ns, k, ks);
  /* call shared interface of ascon encrypt */
  trigger_high();
  crypto_aead_encrypt_shared(cs, clen, ms, mlen, as, alen, ns, ks);
  trigger_low();
  /* unmask shared output data */
  combine_shares_encrypt(cs, c, *clen);
  /* free shares */
  free(ks);
  free(ns);
  free(as);
  free(ms);
  free(cs);
  printbytes("c", c, mlen);
  printbytes("t", c + mlen, CRYPTO_ABYTES);
  print("\n");
  return 0;
}

int crypto_aead_decrypt(unsigned char* m, unsigned long long* mlen,
                        unsigned char* nsec, const unsigned char* c,
                        unsigned long long clen, const unsigned char* a,
                        unsigned long long alen, const unsigned char* npub,
                        const unsigned char* k) {
  int result = 0;
  (void)nsec;
  print("decrypt\n");
  printbytes("k", k, CRYPTO_KEYBYTES);
  printbytes("n", npub, CRYPTO_NPUBBYTES);
  printbytes("a", a, alen);
  printbytes("c", c, clen - CRYPTO_ABYTES);
  printbytes("t", c + clen - CRYPTO_ABYTES, CRYPTO_ABYTES);
  if (clen < CRYPTO_ABYTES) return -1;
  /* dynamic allocation of input/output shares */
  mask_key_uint32_t* ks = malloc(sizeof(*ks) * NUM_WORDS(CRYPTO_KEYBYTES));
  mask_npub_uint32_t* ns = malloc(sizeof(*ns) * NUM_WORDS(CRYPTO_NPUBBYTES));
  mask_ad_uint32_t* as = malloc(sizeof(*as) * NUM_WORDS(alen));
  mask_m_uint32_t* ms = malloc(sizeof(*ms) * NUM_WORDS(clen - CRYPTO_ABYTES));
  mask_c_uint32_t* cs = malloc(sizeof(*cs) * NUM_WORDS(clen));
  /* mask plain input data */
  generate_shares_decrypt(c, cs, clen, a, as, alen, npub, ns, k, ks);
  /* call shared interface of ascon decrypt */
  trigger_high();
  result = crypto_aead_decrypt_shared(ms, mlen, cs, clen, as, alen, ns, ks);
  trigger_low();
  /* unmask shared output data */
  combine_shares_decrypt(ms, m, *mlen);
  /* free shares */
  free(ks);
  free(ns);
  free(as);
  free(ms);
  free(cs);
  printbytes("m", m, *mlen);
  print("\n");
  return result;
}
