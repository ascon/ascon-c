#ifndef ASCON_H_
#define ASCON_H_

#include <stdint.h>

#include "word.h"

typedef struct {
  word_t x0, x1, x2, x3, x4;
} state_t;

#endif /* ASCON_H */
