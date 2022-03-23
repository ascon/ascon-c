#!/bin/bash

RESULTFILE=$1
shift
FACTOR=$1

rm -f $RESULTFILE

echo -e "  size \t|     1 |     8 |    16 |    32 |    64 |  1536 |  long | implementation"
echo -e  "-------:|------:|------:|------:|------:|------:|------:|------:|:---------------"
for i in getcycles_*; do
  CYCLES=$(./$i $FACTOR | tail -n 1)
  LIB=$(echo $i | sed 's/getcycles_/lib/')
  SIZE=$(size -t ${LIB}.a | grep TOTALS | awk '{print $1}')
  echo -e " $SIZE \t$CYCLES $i" | grep -v "^0"
  echo -e " $SIZE \t$CYCLES $i" | grep -v "^0" >> $RESULTFILE
done

sort -n -k7 -t\| -o $RESULTFILE $RESULTFILE

echo -e "  size \t|     1 |     8 |    16 |    32 |    64 |  1536 |  long | implementation\n-------:|------:|------:|------:|------:|------:|------:|------:|:---------------\n$(cat $RESULTFILE)" > $RESULTFILE

