#!/bin/bash

ALG="ascon128 ascon128a ascon80pq asconxof asconxofa asconprf asconprfs ascon ascona"
ARCH="x86-64-v4 armv7-a armv7-m armv6-m rv32i avr"

printf "| %-9s "
for a in $ALG; do
  printf "| %-9s " $a
done
echo "|"

echo -n "|:----------"
for a in $ALG; do
  echo -n "|----------:"
done
echo "|"

for i in $ARCH; do
  printf "| %-9s " $i
  for a in $ALG; do
    printf "| %-9s " $(grep -h O size/${a}_*_${i}.md 2>/dev/null | cut -d\| -f1 | sort -n | head -n1)
  done
  echo "|"
done
