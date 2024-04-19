#!/bin/bash

if [ -z $1 ]; then
  echo "Usage: $0 <algorithms>..."
  exit 1
fi

CC="arm-none-eabi-gcc"
CFLAGS="-mcpu=cortex-m0"
ALG=$*

for a in $ALG; do
  for i in crypto_*/${a}v12/*armv6m*; do
    for o in s 0 1 2 3; do
      if $CC $CFLAGS -O$o -Itests -I$i $i/*.[cS] -c 2>/dev/null; then
        echo -n "[PASS] "
      else
        echo -n "[FAIL] "
      fi
      echo "$CC $CFLAGS -O$o -Itests -I$i $i/*.[cS]"
      # $CC $CFLAGS -O$o -Itests -I$i tests/genkat_*.c -c
      # $CC $CFLAGS -O$o --specs=nosys.specs -Wl,--gc-sections *.o
    done | grep --color -e $ -e ".*FAIL.*"
  done
done
