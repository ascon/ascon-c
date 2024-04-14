#!/bin/bash

ALG=$1
shift
CC=$1
shift
CFLAGS=$*

for i in crypto_*/${ALG}v12/*/; do
  if $CC $CFLAG -Itests -I$i $i/*.[cS] -c 2>/dev/null; then
    echo -n "[PASS] "
  else
    echo -n "[FAIL] "
  fi
  echo $i
done | grep --color -e $ -e ".*FAIL.*"
