#!/bin/bash

echo -e "  size \t| implementation\n-------:|:---------------"
for LIB in libcrypto_*; do
  SIZE=$(size -t ${LIB} | grep TOTALS | awk '{print $1}')
  echo -e " $SIZE\t| $LIB" | grep -v "^0"
done | sort -k2 -t\| | sort -n -k1 -t\|

