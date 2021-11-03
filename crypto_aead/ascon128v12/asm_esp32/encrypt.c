#include "ascon.h"

int crypto_aead_encrypt(unsigned char* c, unsigned long long* clen,
                        const unsigned char* m, unsigned long long mlen,
                        const unsigned char* ad, unsigned long long adlen,
                        const unsigned char* nsec, const unsigned char* npub,
                        const unsigned char* k) {

    *clen = mlen + CRYPTO_ABYTES;
    (void)nsec;

    return ascon_core(c, m, mlen, ad, adlen, npub, k, 1);
}
