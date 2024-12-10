int crypto_hash(unsigned char *out, const unsigned char *in,
                unsigned long long inlen);
int crypto_xof(unsigned char *out, unsigned long long outlen,
               const unsigned char *in, unsigned long long inlen);
int crypto_cxof(unsigned char *out, unsigned long long outlen,
                const unsigned char *in, unsigned long long inlen,
                const unsigned char *cs, unsigned long long cslen);
