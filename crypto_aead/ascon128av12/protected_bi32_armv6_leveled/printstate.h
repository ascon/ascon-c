#ifndef PRINTSTATE_H_
#define PRINTSTATE_H_

#ifdef ASCON_PRINT_STATE

#include "ascon.h"
#include "word.h"

void print(const char* text);
void printbytes(const char* text, const uint8_t* b, uint64_t len);
void printword(const char* text, const word_t x, int ns);
void printstate(const char* text, const ascon_state_t* si, int ns);

#else

#define print(text) \
  do {              \
  } while (0)

#define printbytes(text, b, len) \
  do {                           \
  } while (0)

#define printword(text, w, ns) \
  do {                         \
  } while (0)

#define printstate(text, s, ns) \
  do {                          \
  } while (0)

#endif

#endif /* PRINTSTATE_H_ */
