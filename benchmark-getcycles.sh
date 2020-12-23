#!/bin/sh

FACTOR=$1
ALG=$2

for i in getcycles*${ALG}v12*; do
  echo
  echo $i:
  echo
  echo "|     1 |     8 |    16 |    32 |    64 |  1536 |  long |"
  echo "|------:|------:|------:|------:|------:|------:|------:|"
  for n in $(seq 5); do
    ./$i $FACTOR | tail -n 1
  done | sort -n -k7 -t\|
done 2>/dev/null
echo
