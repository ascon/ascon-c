#!/bin/sh

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
  armv7m_small \
  armv7m_lowsize \
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
  "

for alg in $ALG_LIST; do
  for impl in $IMPL_LIST; do
    echo "$alg - $impl"
    mkdir -p $alg/$impl
    for i in $(ls $base/$impl | grep -v api.h); do
      cmp --silent $base/$impl/$i $alg/$impl/$i || cp $base/$impl/$i $alg/$impl
    done
    cmp --silent $alg/ref/api.h $alg/$impl/api.h || cp $alg/ref/api.h $alg/$impl
  done
  echo
done

rm -f crypto_*/ascon128*v12/*/hash.c
rm -f crypto_*/ascon80pqv12/*/hash.c
rm -f crypto_*/asconhash*v12/*/aead.c
rm -f crypto_*/asconxof*v12/*/aead.c
rm -f crypto_*/asconhash*v12/*/update*.c
rm -f crypto_*/asconxof*v12/*/update*.c
rm -f crypto_*/asconhash*v12/*/crypto_aead.c
rm -f crypto_*/asconxof*v12/*/crypto_aead.c

rm -f crypto_aead*/*/*/prf*.c
rm -f crypto_hash*/*/*/prf*.c
rm -f crypto_auth/asconmacv12/*/prfs.c
rm -f crypto_auth/asconprfv12/*/prfs.c
rm -f crypto_auth/asconprfsv12/*/prf.c
rm -f crypto_auth/*/*/hash.c
rm -f crypto_auth/*/*/aead.c
rm -rf crypto_auth/*/*lowsize
for i in crypto_auth/*/*/ascon.h; do
  git restore -q $i
done

rm -rf crypto_*/*bi32*/arm*
rm -rf crypto_*/*bi32*/bi8*
rm -rf crypto_*/*bi32*/opt*

sed -i 's/ASCON_EXTERN_BI 0/ASCON_EXTERN_BI 1/' crypto_*/*bi32*/*/config.h

#cp crypto_aead/ascon128v12/neon/* crypto_aead/ascon128av12/neon
#cp crypto_aead/ascon128v12/avx512/* crypto_aead/ascon128av12/avx512
#git checkout crypto_aead/ascon128av12/neon/aead.c
#git checkout crypto_aead/ascon128av12/avx512/aead.c

