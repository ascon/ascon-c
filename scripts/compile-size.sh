#!/bin/bash

cc=$1
flags=$2

for a in ascon128 ascon128a asconhash asconhasha ascon ascona; do
  echo -e "  size \t| opt | implementation\n-------:|-----|:---------------"
  for i in crypto_*/${a}v12/*/; do
    for o in s 0 1 2 3; do
      rm -rf *.o
      $cc $flags -O$o -Itests -I$i $i/*.[cS] -c 2>/dev/null
      [ $? -eq 0 ] && echo -e $(size -t *.o 2>/dev/null | tail -n1 | awk '{print $1}') "\t| -O$o | $i"
    done
  done | sort -k2 -t\| | sort -n -k1 -t\| | head -n5
  echo
done
