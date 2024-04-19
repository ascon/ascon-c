#!/bin/bash

if [ -z $1 ]; then
  echo "Usage: $0 <algorithm> <compiler> [cflags]..."
  exit 1
fi

ALG=$1
shift
CC=$1
shift
CFLAGS=$*

for i in crypto_*/${ALG}v12/*/; do
  if $CC $CFLAGS -Itests -I$i $i/*.[cS] -c 2>/dev/null; then
    echo -n "[PASS] "
  else
    echo -n "[FAIL] "
  fi
  echo $i
done | grep --color -e $ -e ".*FAIL.*"
