#!/bin/bash

FACTOR=$1
shift
RESULTFILE=$1
shift
ALG=$1
shift
IMPL="$*"

echo "FACTOR: $FACTOR"
echo "RESULTFILE: $RESULTFILE"
echo "ALGORITHM: $ALG"
echo "IMPLEMENTATIONS: $IMPL"
echo

rm $RESULTFILE
cmake . -DALG_LIST=$ALG -DTEST_LIST=getcycles

for I in $IMPL; do
  for M in 0 1; do
    for P in 0 1; do
      for U in 0 1; do
        for B in 0 1; do
          rm getcycles_*
          rm *.a
          cmake . -DIMPL_LIST=${I} -DCOMPILE_DEFS="-DASCON_INLINE_MODE=${M};-DASCON_INLINE_PERM=${P};-DASCON_UNROLL_LOOPS=${U};-DASCON_INLINE_BI=${B}"
          cmake --build .
          CYCLES=$(./getcycles_* $FACTOR | tail -n 1)
          SIZE=$(size -t *.a | grep TOTALS | awk '{print $1}')
          echo -e " $SIZE \t$CYCLES IM=${M} IP=${P} UL=${U} IB=${B} | $I" | grep -v "^0" >> $RESULTFILE
        done
      done
    done
  done
done

sort -n -k8 -t\| -o $RESULTFILE $RESULTFILE
echo -e "  size \t|     1 |     8 |    16 |    32 |    64 |  1536 |  long |        config       | implementation\n-------:|------:|------:|------:|------:|------:|------:|------:|:-------------------:|:---------------\n$(cat $RESULTFILE)" > $RESULTFILE
echo
cat $RESULTFILE

