#ifndef RANDOMBYTES_H_
#define RANDOMBYTES_H_

#include <stdlib.h>

#include "forceinline.h"

forceinline void randombytes(unsigned char* x, unsigned long long len) {
  for (unsigned long long i = 0; i < len; ++i) x[i] = rand();
}

#endif /* RANDOMBYTES_H_ */
