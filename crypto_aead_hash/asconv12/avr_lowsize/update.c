#include "api.h"
#include "ascon.h"
#include "permutations.h"
#include "printstate.h"

#if defined(ASCON_AEAD_RATE)
#if ASCON_AEAD_RATE == ASCON_128_RATE
#define ASCON_AEAD_ROUNDS ASCON_128_PB_ROUNDS
#else
#define ASCON_AEAD_ROUNDS ASCON_128A_PB_ROUNDS
#endif
#endif

void ascon_update(uint8_t mode, ascon_state_t* s, uint8_t* c, const uint8_t* m,
                  uint64_t mlen) {
#if defined(ASCON_AEAD_RATE) && !defined(ASCON_HASH_ROUNDS)
  const int nr = ASCON_AEAD_ROUNDS;
  const int rate = ASCON_AEAD_RATE;
#elif !defined(ASCON_AEAD_RATE) && defined(ASCON_HASH_ROUNDS)
  const int nr = ASCON_HASH_ROUNDS;
  const int rate = ASCON_HASH_RATE;
#else
  const int nr = (mode & ASCON_HASH) ? ASCON_HASH_ROUNDS : ASCON_AEAD_ROUNDS;
  const int rate = (mode & ASCON_HASH) ? ASCON_HASH_RATE : ASCON_AEAD_RATE;
#endif
  /* full plaintext blocks */
  while (mlen >= rate) {
    if (mode == 1 || mode == 9) ABSORB(s->b[0], m, 8);
#if defined(ASCON_HASH_ROUNDS)
    if (mode == 2 || mode == 10) SQUEEZE(c, s->b[0], 8);
#endif
#if defined(ASCON_AEAD_RATE)
    if (mode == 3) ENCRYPT(s->b[0], c, m, 8);
    if (mode == 7) DECRYPT(s->b[0], c, m, 8);
    if (mode == 1 && ASCON_AEAD_RATE == 16) ABSORB(s->b[1], m + 8, 8);
    if (mode == 3 && ASCON_AEAD_RATE == 16) ENCRYPT(s->b[1], c + 8, m + 8, 8);
    if (mode == 7 && ASCON_AEAD_RATE == 16) DECRYPT(s->b[1], c + 8, m + 8, 8);
#endif
    if (mode == 1) printstate("absorb adata", s);
    if (mode == 3) printstate("absorb plaintext", s);
    if (mode == 7) printstate("insert ciphertext", s);
    if (mode == 8) printstate("squeeze output", s);
#if defined(ASCON_HASH_ROUNDS)
    if (!(mode == 10 && mlen == rate)) P(s, nr);
#else
    P(s, nr);
#endif
    m += rate;
    c += rate;
    mlen -= rate;
  }
  /* final plaintext block */
  int i = 0;
#if defined(ASCON_AEAD_RATE)
  if (ASCON_AEAD_RATE == 16 && mlen >= 8) {
    if (mode == 1) ABSORB(s->b[0], m, 8);
    if (mode == 3) ENCRYPT(s->b[0], c, m, 8);
    if (mode == 7) DECRYPT(s->b[0], c, m, 8);
    i = 1;
    m += 8;
    c += 8;
    mlen -= 8;
  }
  if (mode == 3) ENCRYPT(s->b[i], c, m, mlen);
  if (mode == 7) DECRYPT(s->b[i], c, m, mlen);
#endif
  if (mode == 1 || mode == 9) ABSORB(s->b[i], m, mlen);
#if defined(ASCON_HASH_ROUNDS)
  if (mode == 2 || mode == 10) SQUEEZE(c, s->b[0], mlen);
#endif
  s->b[i][7 - mlen] ^= 0x80;
  if (mode == 1) printstate("pad adata", s);
  if (mode == 3) printstate("pad plaintext", s);
  if (mode == 7) printstate("pad ciphertext", s);
}
