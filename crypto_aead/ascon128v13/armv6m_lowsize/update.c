#include "api.h"
#include "ascon.h"
#include "permutations.h"
#include "printstate.h"

#ifdef ASCON_AEAD_RATE

#if ASCON_AEAD_RATE == ASCON_128_RATE
#define ASCON_AEAD_ROUNDS ASCON_128_PB_ROUNDS
#else
#define ASCON_AEAD_ROUNDS ASCON_128A_PB_ROUNDS
#endif

void ascon_update(ascon_state_t* s, uint8_t* out, const uint8_t* in,
                  uint64_t len, uint8_t mode) {
#if defined(ASCON_HASH_BYTES)
  const int nr = (mode & ASCON_HASH) ? ASCON_HASH_ROUNDS : ASCON_AEAD_ROUNDS;
  const int rate = (mode & ASCON_HASH) ? ASCON_HASH_RATE : ASCON_AEAD_RATE;
#else
  const int nr = ASCON_AEAD_ROUNDS;
  const int rate = ASCON_AEAD_RATE;
#endif
#if ASCON_AEAD_RATE == 8
  const int i = 0;
  (void)rate;
#else
  int i = 0;
#endif
  uint64_t tmp = 0;
  while (len >= 8) {
    /* absorb data */
#ifdef ASCON_HASH_BYTES
    if (mode & ASCON_ABSORB)
#endif
    {
      tmp = LOAD(in, 8);
      s->x[i] ^= tmp;
      if (mode == ASCON_ABSORB) printstate("absorb adata", s);
      if (mode == ASCON_ENCRYPT) printstate("absorb plaintext", s);
    }
    /* extract data */
    if (mode & ASCON_SQUEEZE) {
      STORE(out, s->x[i], 8);
      if (mode & ASCON_HASH) printstate("squeeze output", s);
    }
    /* insert data */
    if (mode & ASCON_INSERT) {
      s->x[i] = tmp;
      printstate("insert ciphertext", s);
    }
    /* compute permutation for full blocks */
#if ASCON_AEAD_RATE == 16
    if (++i == rate / 8) i = 0;
#endif
    if (i == 0) P(s, nr);
    in += 8;
    out += 8;
    len -= 8;
  }
  /* absorb data */
#ifdef ASCON_HASH_BYTES
  if (mode & ASCON_ABSORB)
#endif
  {
    tmp = LOADBYTES(in, len);
    s->x[i] ^= tmp;
  }
  /* extract data */
  if (mode & ASCON_SQUEEZE) STOREBYTES(out, s->x[i], len);
  /* insert data */
  if (mode & ASCON_INSERT) {
    s->x[i] = CLEAR(s->x[i], len);
    s->x[i] ^= tmp;
  }
  s->x[i] ^= PAD(len);
}

#endif
