#include "ascon.h"

int crypto_aead_decrypt(unsigned char* m, unsigned long long* mlen,
                        unsigned char* nsec, const unsigned char* c,
                        unsigned long long clen, const unsigned char* ad,
                        unsigned long long adlen, const unsigned char* npub,
                        const unsigned char* k) {
    if (clen < CRYPTO_ABYTES) {
        *mlen = 0;
        return -1;
    }

    *mlen = clen - CRYPTO_ABYTES;
    (void)nsec;

    return ascon_core(m, c, *mlen, ad, adlen, npub, k, -1);
}
