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

echo -e "  size \t| opt | implementation\n-------:|-----|:---------------"
for i in crypto_*/${ALG}v12/*/; do
  for o in z s 1 2; do
    rm -rf *.o
    $CC $CFLAGS -O$o -Itests -I$i $i/*.[cS] -c 2>/dev/null
    [ $? -eq 0 ] && echo -e $(size -t *.o 2>/dev/null | tail -n1 | awk '{print $1}') "\t| -O$o | $i"
  done
done | sort -k2 -t\| | sort -n -k1 -t\| | sed 's|/$||'
echo
