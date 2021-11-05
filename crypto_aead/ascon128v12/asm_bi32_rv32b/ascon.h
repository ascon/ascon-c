#include "api.h"

int ascon_core(unsigned char * outptr,
               const unsigned char * inptr, unsigned int inlen,
               const unsigned char * adptr, unsigned int adlen,
               const unsigned char * nptr, const unsigned char * kptr,
               unsigned char mode);
