#!/bin/bash

base="src"

ALG_LIST=" \
  crypto_aead/ascon128v12 \
  crypto_aead/ascon128av12 \
  crypto_aead/ascon80pqv12 \
  crypto_hash/asconhashv12 \
  crypto_hash/asconhashav12 \
  crypto_hash/asconxofv12 \
  crypto_hash/asconxofav12 \
  crypto_auth/asconmacv12 \
  crypto_auth/asconmacav12 \
  crypto_auth/asconprfv12 \
  crypto_auth/asconprfav12 \
  crypto_auth/asconprfsv12 \
  crypto_aead_hash/asconv12 \
  crypto_aead_hash/asconav12 \
  crypto_aead/ascon128bi32v12 \
  crypto_aead/ascon128abi32v12 \
  crypto_hash/asconhashbi32v12 \
  crypto_hash/asconhashabi32v12 \
  crypto_aead_hash/asconbi32v12 \
  crypto_aead_hash/asconabi32v12 \
  "

IMPL_LIST=" \
  armv6 \
  armv6_lowsize \
  armv6m \
  armv6m_lowsize \
  armv7m \
  armv7m_lowsize \
  armv7m_small \
  avr \
  avr_lowsize \
  bi32 \
  bi32_armv6 \
  bi32_armv6m \
  bi32_armv7m \
  bi32_armv7m_small \
  bi32_lowreg \
  bi32_lowsize \
  bi8 \
  opt32 \
  opt32_lowsize \
  opt64 \
  opt64_lowsize \
  opt8 \
  opt8_lowsize \
  "

COMMON_FILES=" \
  aead.c \
  ascon.h \
  constants.h \
  bendian.h \
  forceinline.h \
  goal-constbranch \
  goal-constindex \
  hash.c \
  implementors \
  permutations.c \
  permutations.h \
  prf.c \
  prfs.c \
  printstate.c \
  printstate.h \
  word.h \
  "

LOWSIZE_FILES=" \
  aead.c \
  ascon.h \
  config.h \
  crypto_aead.c \
  "

COMBINED_LOWSIZE_FILES=" \
  hash.c \
  "

BI32_FILES=" \
  constants.c \
  constants.h \
  interleave.c \
  interleave.h \
  word.h \
  "

ARM_FILES=" \
  architectures \
  "

OPT8_FILES=" \
  aead.c \
  config.h \
  hash.c \
  prf.c \
  prfs.c \
  round.h \
  word.h \
  "

OPT8_LOWSIZE_FILES=" \
  aead.c \
  ascon.h \
  hash.c \
  update.c \
  "

AVR_FILES=" \
  implementors \
  permutations.S \
  "

for alg in $ALG_LIST; do
  for impl in $IMPL_LIST; do
    echo
    echo "$alg - $impl"
    mkdir -p $alg/$impl
    for i in $COMMON_FILES; do
      a=$base/$i
      b=$alg/$impl/$i
      echo "  cp $a $b"
      cmp --silent $a $b || cp $a $b
    done
    if [[ $impl == *"lowsize"* ]]; then
      for i in $LOWSIZE_FILES; do
        a=$base/lowsize/$i
        b=$alg/$impl/$i
        echo "  cp $a $b"
        cmp --silent $a $b || cp $a $b
      done
    fi
    if [[ $impl == *"lowsize"* && $alg != *"crypto_hash"* ]]; then
      a=$base/lowsize/update.c
      b=$alg/$impl/update.c
      echo "  cp $a $b"
      cmp --silent $a $b || cp $a $b
    fi
    if [[ $impl == *"bi32"* ]]; then
      for i in $BI32_FILES; do
        a=$base/bi32/$i
        b=$alg/$impl/$i
        echo "  cp $a $b"
        cmp --silent $a $b || cp $a $b
      done
    fi
    if [[ $impl == *"arm"* ]]; then
      for i in $ARM_FILES; do
        a=$base/arm/$i
        b=$alg/$impl/$i
        echo "  cp $a $b"
        cmp --silent $a $b || cp $a $b
      done
    fi
    if [[ $impl == *"avr"* ]]; then
      for i in $AVR_FILES; do
        a=$base/avr/$i
        b=$alg/$impl/$i
        echo "  cp $a $b"
        cmp --silent $a $b || cp $a $b
      done
    fi
    if [[ $impl == *"opt8"* || $impl == *"avr"* ]]; then
      for i in $OPT8_FILES; do
        a=$base/opt8/$i
        b=$alg/$impl/$i
        echo "  cp $a $b"
        cmp --silent $a $b || cp $a $b
      done
    fi
    if [[ $impl == *"opt8_lowsize"* || $impl == *"avr_lowsize"* ]]; then
      for i in $OPT8_LOWSIZE_FILES; do
        a=$base/opt8_lowsize/$i
        b=$alg/$impl/$i
        echo "  cp $a $b"
        cmp --silent $a $b || cp $a $b
      done
    fi
    if [[ $impl == *"lowsize"* && $alg == *"crypto_aead_hash"* ]]; then
      for i in $COMBINED_LOWSIZE_FILES; do
        a=$base/combined/$i
        b=$alg/$impl/$i
        echo "  cp $a $b"
        cmp --silent $a $b || cp $a $b
      done
    fi
    for i in $(ls $base/$impl 2>/dev/null); do
      a=$base/$impl/$i
      b=$alg/$impl/$i
      echo "  cp $a $b"
      cmp --silent $a $b || cp $a $b
    done
    a=$alg/ref/api.h
    b=$alg/$impl/api.h
    echo "  cp $a $b"
    cmp --silent $a $b || cp $a $b
  done
  echo
done

rm -f crypto_*/ascon128*v12/*/hash.c
rm -f crypto_*/ascon80pqv12/*/hash.c
rm -f crypto_*/asconhash*v12/*/aead.c
rm -f crypto_*/asconxof*v12/*/aead.c
rm -f crypto_*/asconhash*v12/*/crypto_aead.c
rm -f crypto_*/asconxof*v12/*/crypto_aead.c

rm -f crypto_aead*/*/*/prf*.c
rm -f crypto_hash*/*/*/prf*.c
rm -f crypto_auth/asconmacv12/*/prfs.c
rm -f crypto_auth/asconmacav12/*/prfs.c
rm -f crypto_auth/asconprfv12/*/prfs.c
rm -f crypto_auth/asconprfav12/*/prfs.c
rm -f crypto_auth/asconprfsv12/*/prf.c
rm -f crypto_auth/*/*/hash.c
rm -f crypto_auth/*/*/aead.c
rm -f crypto_*/*/opt8_lowsize/crypto_aead.c
rm -f crypto_*/*/avr_lowsize/crypto_aead.c
rm -f crypto_*/*/avr*/permutations.c
rm -rf crypto_auth/*/*lowsize

rm -rf crypto_*/*bi32*/arm*
rm -rf crypto_*/*bi32*/bi8*
rm -rf crypto_*/*bi32*/opt*
rm -rf crypto_*/*bi32*/avr*

rm -rf crypto_*/*/avr/permutations.c

sed -i 's/ASCON_EXTERN_BI 0/ASCON_EXTERN_BI 1/' crypto_*/*bi32*/*/config.h

