#!/bin/bash

if [ -z $1 ]; then
  echo "Usage: $0 <algorithms>..."
  exit 1
fi

ALG=$*
ARCH="x86-64-v4 armv7-a armv7-m armv6-m rv32i esp32 avr"

printf "| %-15s "
for a in $ALG; do
  printf "| %-15s " $a
done
echo "|"

echo -n "|:----------------"
for a in $ALG; do
  echo -n "|----------------:"
done
echo "|"

for i in $ARCH; do
  printf "| %-15s " $i
  for a in $ALG; do
    printf "| %-15s " $(grep -h O size/${a}_*_${i}.md 2>/dev/null | cut -d\| -f1 | sort -n | head -n1)
  done
  echo "|"
done
