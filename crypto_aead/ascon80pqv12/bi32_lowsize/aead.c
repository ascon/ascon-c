#include "api.h"
#include "ascon.h"
#include "permutations.h"
#include "printstate.h"

forceinline void ascon_loadkey(word_t* K0, word_t* K1, word_t* K2,
                               const uint8_t* k) {
  KINIT(K0, K1, K2);
  if (CRYPTO_KEYBYTES == 16) {
    *K1 = XOR(*K1, LOAD(k, 8));
    *K2 = XOR(*K2, LOAD(k + 8, 8));
  }
  if (CRYPTO_KEYBYTES == 20) {
    *K0 = XOR(*K0, KEYROT(WORD_T(0), LOADBYTES(k, 4)));
    *K1 = XOR(*K1, LOADBYTES(k + 4, 8));
    *K2 = XOR(*K2, LOADBYTES(k + 12, 8));
  }
}

forceinline void ascon_aeadinit(state_t* s, const uint8_t* npub, word_t K0,
                                word_t K1, word_t K2) {
  if (CRYPTO_KEYBYTES == 16 && ASCON_AEAD_RATE == 8) s->x0 = ASCON_128_IV;
  if (CRYPTO_KEYBYTES == 16 && ASCON_AEAD_RATE == 16) s->x0 = ASCON_128A_IV;
  if (CRYPTO_KEYBYTES == 20) s->x0 = ASCON_80PQ_IV;
  if (CRYPTO_KEYBYTES == 20) s->x0 = XOR(s->x0, K0);
  s->x1 = K1;
  s->x2 = K2;
  s->x3 = LOAD(npub, 8);
  s->x4 = LOAD(npub + 8, 8);
  P(s, 12);
  if (CRYPTO_KEYBYTES == 20) s->x2 = XOR(s->x2, K0);
  s->x3 = XOR(s->x3, K1);
  s->x4 = XOR(s->x4, K2);
  printstate("initialization", s);
}

forceinline void ascon_final(state_t* s, word_t K0, word_t K1, word_t K2) {
  if (CRYPTO_KEYBYTES == 16 && ASCON_AEAD_RATE == 8) {
    s->x1 = XOR(s->x1, K1);
    s->x2 = XOR(s->x2, K2);
  }
  if (CRYPTO_KEYBYTES == 16 && ASCON_AEAD_RATE == 16) {
    s->x2 = XOR(s->x2, K1);
    s->x3 = XOR(s->x3, K2);
  }
  if (CRYPTO_KEYBYTES == 20) {
    s->x1 = XOR(s->x1, KEYROT(K0, K1));
    s->x2 = XOR(s->x2, KEYROT(K1, K2));
    s->x3 = XOR(s->x3, KEYROT(K2, WORD_T(0)));
  }
  P(s, 12);
  s->x3 = XOR(s->x3, K1);
  s->x4 = XOR(s->x4, K2);
  printstate("finalization", s);
}

void ascon_aead(state_t* s, uint8_t* out, const uint8_t* in, uint64_t tlen,
                const uint8_t* ad, uint64_t adlen, const uint8_t* npub,
                const uint8_t* k, uint8_t mode) {
  const int nr = (ASCON_AEAD_RATE == 8) ? 6 : 8;
  word_t K0, K1, K2;
  ascon_loadkey(&K0, &K1, &K2, k);
  /* initialize */
  ascon_aeadinit(s, npub, K0, K1, K2);
  /* process associated data */
  if (adlen) {
    ascon_update(s, (void*)0, ad, adlen, ASCON_ABSORB);
    P(s, nr);
  }
  /* domain separation */
  s->x4 = XOR(s->x4, WORD_T(1));
  printstate("process associated data", s);
  /* process plaintext/ciphertext */
  ascon_update(s, out, in, tlen, mode);
  if (mode == ASCON_ENCRYPT) printstate("process plaintext", s);
  if (mode == ASCON_DECRYPT) printstate("process ciphertext", s);
  /* finalize */
  ascon_final(s, K0, K1, K2);
}
