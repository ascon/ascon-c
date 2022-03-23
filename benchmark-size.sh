#!/bin/bash

RESULTFILE=$1

rm $RESULTFILE

for LIB in libcrypto_*; do
  SIZE=$(size -t ${LIB} | grep TOTALS | awk '{print $1}')
  echo -e " $SIZE\t| $LIB" | grep -v "^0" >> $RESULTFILE
done

sort -k2 -t\| -o $RESULTFILE $RESULTFILE
sort -n -k1 -t\| -o $RESULTFILE $RESULTFILE

echo -e "  size \t| implementation\n-------:|:---------------\n$(cat $RESULTFILE)" > $RESULTFILE

